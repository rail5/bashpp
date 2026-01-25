/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/DidChangeTextDocumentNotification.h"

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

	// Debounce didChange processing
	uint64_t now = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()); // Record current timestamp
	{
		std::lock_guard<std::mutex> lock(debounce_mutex);
		if (!debounce_timestamps.count(uri)) {
			debounce_timestamps[uri] = std::make_shared<std::atomic<uint64_t>>(0);
		}
		debounce_timestamps[uri]->store(now); // Store the previously-recorded timestamp in the map
	}

	std::thread([this, uri, now, did_change_notification]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100ms debounce
		bool should_process = false;
		{
			std::lock_guard<std::mutex> lock(debounce_mutex);
			if (debounce_timestamps[uri]->load() == now) {
				should_process = true; // Only re-parse if this thread's "now" is the last one that was stored in the map
				// Earlier threads had their timestamps overwritten and will not continue
			}
		}
		if (!should_process) return; // Another thread will handle this

		// Update the "unsaved changes" stored
		program_pool.set_unsaved_file_contents(uri, std::get<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0)).text);

		log("Re-parsing program for URI: ", uri);
		// Per the LSP spec, contentChanges is an array of either:
		// 1. TextDocumentContentChangeWholeDocument
		// 2. TextDocumentContentChangePartial

		// At the moment, we only support whole document changes
		// The logic to support partial changes probably won't be too complicated altogether,
		// But regardless, that's for later.

		// We'll also only handle the case where there is exactly one change
		// If there are multiple changes, we will ignore them for now

		// TODO(@rail5): Handling partial changes and handling multiple changes (possibly of different types) **must** be implemented in the future
		auto program = program_pool.re_parse_program(uri);
		if (program != nullptr) {
			publishDiagnostics(program);
		} else {
			log("Failed to re-parse program: ", uri);
		}
	}).detach();
}
