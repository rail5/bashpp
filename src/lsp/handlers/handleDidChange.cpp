/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidChangeTextDocumentNotification.h>

void bpp::BashppServer::handleDidChange(const GenericNotificationMessage& request) {
	DidChangeTextDocumentNotification did_change_notification = request.toSpecific<DidChangeTextDocumentParams>();
	std::string uri = did_change_notification.params.textDocument.uri;

	log("Received DidChange notification for URI: ", uri);

	// Ensure the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to re-parse non-local file: ", uri);
		return;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	if (did_change_notification.params.contentChanges.size() != 1) {
		// For now, we only support single content changes
		log("Ignoring DidChange notification for URI: ", uri, " as it has multiple content changes.");
		return;
	}

	if (!std::holds_alternative<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0))) {
		// For now, we only support whole document changes
		log("Ignoring DidChange notification for URI: ", uri, " as its content change is not a whole document change.");
		return;
	}

	processing_didChange.store(true, std::memory_order_release);

	const std::string main_program_uri = program_pool.get_program(uri)->get_main_source_file();
	const std::string new_content = std::get<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0)).text;
	std::shared_ptr<DebounceState> debounce_state;
	uint64_t change_generation_for_this_thread = 0;
	uint32_t debounce_time_in_milliseconds = 100;

	debounce_state = debounce_states.get(main_program_uri);
	change_generation_for_this_thread =
		debounce_state->change_generation.fetch_add(1, std::memory_order_acq_rel) + 1;
	
	debounce_time_in_milliseconds =
		debounce_state->debounce_time_in_milliseconds.load(std::memory_order_acquire);

	std::thread([this,
		uri,
		main_program_uri,
		debounce_state,
		change_generation_for_this_thread,
		debounce_time_in_milliseconds,
		new_content]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(debounce_time_in_milliseconds));
			// Only proceed if no newer change generation has been recorded
			if (debounce_state->change_generation.load(std::memory_order_acquire) != change_generation_for_this_thread) {
				return; // A newer change has been recorded, slet the newer thread handle it
			}
		
			program_pool.set_unsaved_file_contents(uri, new_content);

			log("Re-parsing program '", main_program_uri, "' due to change in URI: ", uri);

			// Per the LSP spec, contentChanges is an array of either:
			// 1. TextDocumentContentChangeWholeDocument
			// 2. TextDocumentContentChangePartial

			// At the moment, we only support whole document changes
			// The logic to support partial changes probably won't be too complicated altogether,
			// But regardless, that's for later.

			// We'll also only handle the case where there is exactly one change
			// If there are multiple changes, we will ignore them for now

			// TODO(@rail5): Handling partial changes and handling multiple changes (possibly of different types) **must** be implemented in the future
			const auto reparse_start_time = std::chrono::steady_clock::now();
			auto program = program_pool.re_parse_program(main_program_uri);
			const auto reparse_end_time = std::chrono::steady_clock::now();

			if (program != nullptr) {
				publishDiagnostics(program);
			} else {
				log("Failed to re-parse program: ", uri);
				return; // Don't allow failed parses to affect debounce timing
			}

			const uint64_t reparse_duration_in_microseconds =
				static_cast<uint64_t>(
					std::chrono::duration_cast<std::chrono::microseconds>(reparse_end_time - reparse_start_time).count()
				);

			// Integer EWMA update: weight = 1/4 (no floats)
			constexpr uint64_t weight_numerator = 1;
			constexpr uint64_t weight_denominator = 4;

			const uint64_t previous_average_reparse_time_in_microseconds =
				debounce_state->average_reparse_time_in_microseconds.load(std::memory_order_acquire);		
		
			const uint64_t new_average_reparse_time_in_microseconds =
				(previous_average_reparse_time_in_microseconds == 0)
					? reparse_duration_in_microseconds
					: (
						(previous_average_reparse_time_in_microseconds * (weight_denominator - weight_numerator)
						+ reparse_duration_in_microseconds * weight_numerator)
						/ weight_denominator
					  );
		
			debounce_state->average_reparse_time_in_microseconds.store(
				new_average_reparse_time_in_microseconds,
				std::memory_order_release
			);

			// Derive debounce from average
			constexpr uint32_t minimum_debounce_time_in_milliseconds = 25;
			constexpr uint32_t maximum_debounce_time_in_milliseconds = 1000;
			constexpr uint32_t baseline_debounce_time_in_milliseconds = 100;

			// Scale: 3/4
			constexpr uint32_t scale_numerator = 3;
			constexpr uint32_t scale_denominator = 4;

			const uint64_t new_average_reparse_time_in_milliseconds =
				new_average_reparse_time_in_microseconds / 1000;
			
			const uint64_t scaled_component_in_milliseconds =
				(scale_numerator * new_average_reparse_time_in_milliseconds) / scale_denominator;
			
			uint32_t new_debounce_time_in_milliseconds =
				static_cast<uint32_t>(
					baseline_debounce_time_in_milliseconds + scaled_component_in_milliseconds
				);
			
			// Clamp
			new_debounce_time_in_milliseconds = std::clamp(
				new_debounce_time_in_milliseconds,
				minimum_debounce_time_in_milliseconds,
				maximum_debounce_time_in_milliseconds
			);
			debounce_state->debounce_time_in_milliseconds.store(
				new_debounce_time_in_milliseconds,
				std::memory_order_release
			);

			processing_didChange.store(false, std::memory_order_release);
		}).detach();
}
