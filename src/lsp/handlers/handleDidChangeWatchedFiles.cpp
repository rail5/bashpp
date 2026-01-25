/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/DidChangeWatchedFilesNotification.h"

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
		// Ensure the URI starts with "file://"
		std::string uri = change.uri;
		if (uri.find("file://") != 0) {
			log("Ignoring request to re-parse non-local file: ", uri);
			return;
		} else {
			// Strip the "file://" prefix
			uri = uri.substr(7);
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
