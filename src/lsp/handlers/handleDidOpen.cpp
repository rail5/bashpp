/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidOpenTextDocumentNotification.h>
#include <lsp/include/validateUri.h>

void bpp::BashppServer::handleDidOpen(const GenericNotificationMessage& request) {
	DidOpenTextDocumentNotification did_open_notification = request.toSpecific<DidOpenTextDocumentParams>();
	
	log("Received DidOpen notification for URI: ", did_open_notification.params.textDocument.uri);

	// Ensure the URI starts with "file://"
	// We will reject any URIs that do not point to a file
	std::string uri = did_open_notification.params.textDocument.uri;

	try {
		uri = validateUri(did_open_notification.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in DidOpen notification: ", e.what());
		return;
	}

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri);
	log("Finished parsing: ", uri);
	if (program == nullptr) {
		log("Failed to parse program: ", uri);
		return;
	}
	publishDiagnostics(program);
	program_pool.open_file(uri); // Mark the file as open
}
