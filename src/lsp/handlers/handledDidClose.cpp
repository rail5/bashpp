/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidCloseTextDocumentNotification.h>

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
	program_pool.remove_unsaved_file_contents(uri);
	program_pool.close_file(uri); // Mark the file as closed
}
