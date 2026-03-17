/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidCloseTextDocumentNotification.h>
#include <lsp/include/validateUri.h>

void bpp::BashppServer::handleDidClose(const GenericNotificationMessage& request) {
	DidCloseTextDocumentNotification did_close_notification = request.toSpecific<DidCloseTextDocumentParams>();
	std::string uri = did_close_notification.params.textDocument.uri;

	log("Received DidClose notification for URI: ", uri);

	try {
		uri = validateUri(did_close_notification.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in DidClose notification: ", e.what());
		return;
	}

	// If we've stored unsaved changes for this URI, we can remove them
	program_pool.remove_unsaved_file_contents(uri);
	program_pool.close_file(uri); // Mark the file as closed
}
