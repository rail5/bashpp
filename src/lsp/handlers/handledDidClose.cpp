/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/DidCloseTextDocumentNotification.h"

void bpp::BashppServer::handleDidClose(const GenericNotificationMessage& request) {
	DidCloseTextDocumentNotification did_close_notification = request.toSpecific<DidCloseTextDocumentParams>();
	std::string uri = did_close_notification.params.textDocument.uri;

	log("Received DidClose notification for URI: ", uri);

	// Ensure the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to close non-local file: ", uri);
		return;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	// If we've stored unsaved changes for this URI, we can remove them
	{
		std::lock_guard<std::mutex> lock(unsaved_changes_mutex);
		auto it = unsaved_changes.find(uri);
		if (it != unsaved_changes.end()) {
			log("Forgetting unsaved changes for URI: ", uri);
			unsaved_changes.erase(it);
		}
	}

	program_pool.close_file(uri); // Mark the file as closed
}
