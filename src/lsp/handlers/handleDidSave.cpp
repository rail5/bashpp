/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidSaveTextDocumentNotification.h>
#include <lsp/include/validateUri.h>

void bpp::BashppServer::handleDidSave(const GenericNotificationMessage& request) {
	DidSaveTextDocumentNotification did_save_notification = request.toSpecific<DidSaveTextDocumentParams>();

	log("Received DidSave notification for URI: ", did_save_notification.params.textDocument.uri);

	// Ensure the URI starts with "file://"
	// We will reject any URIs that do not point to a file
	std::string uri = did_save_notification.params.textDocument.uri;

	try {
		uri = validateUri(did_save_notification.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in DidSave notification: ", e.what());
		return;
	}

	program_pool.remove_unsaved_file_contents(uri); // Remove unsaved changes for this URI
}
