/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
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
		std::shared_ptr<bpp::bpp_program> program = program_pool.re_parse_program(uri);
		if (program != nullptr) {
			publishDiagnostics(program);
		} else {
			log("Failed to re-parse program: ", uri);
		}
	}
}
