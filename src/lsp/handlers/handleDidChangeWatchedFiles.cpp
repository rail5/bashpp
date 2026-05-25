/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidChangeWatchedFilesNotification.h>
#include <lsp/include/validateUri.h>

void bpp::BashppServer::handleDidChangeWatchedFiles(const GenericNotificationMessage& request) {
	DidChangeWatchedFilesNotification did_change_notification = request.toSpecific<DidChangeWatchedFilesParams>();
	std::vector<FileEvent> changes = did_change_notification.params.changes;
	if (changes.empty()) {
		log("Received empty DidChangeWatchedFiles notification, ignoring.");
		return;
	}
	log("Received DidChangeWatchedFiles notification for ", changes.size(), " files.");
	
	for (const auto& change : changes) {
		log("File change detected: ", change.uri);

		std::string uri = change.uri;

		try {
			uri = validateUri(change.uri);
		} catch (const std::exception& e) {
			log("Invalid URI in DidChangeWatchedFiles notification: ", e.what());
			return;
		}

		program_pool.remove_unsaved_file_contents(uri); // Remove unsaved changes for this URI

		log("Re-parsing program for URI: ", uri);
		std::vector<std::shared_ptr<bpp::bpp_program>> programs = program_pool.re_parse_programs(uri);

		if (programs.empty()) {
			log("Failed to re-parse any programs for URI: ", uri);
			return; // Don't allow failed parses to affect debounce timing
		}

		for (const auto& program : programs) {
			if (program != nullptr) {
				publishDiagnostics(program);
			} else {
				log("Failed to re-parse a program for URI: ", uri);
			}
		}
	}
}
