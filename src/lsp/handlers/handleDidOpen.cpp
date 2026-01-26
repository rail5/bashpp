/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DidOpenTextDocumentNotification.h>

void bpp::BashppServer::handleDidOpen(const GenericNotificationMessage& request) {
	DidOpenTextDocumentNotification did_open_notification = request.toSpecific<DidOpenTextDocumentParams>();
	
	log("Received DidOpen notification for URI: ", did_open_notification.params.textDocument.uri);

	// Ensure the URI starts with "file://"
	// We will reject any URIs that do not point to a file
	std::string uri = did_open_notification.params.textDocument.uri;
	if (uri.find("file://") != 0) {
		log("Ignoring request to parse non-local file: ", uri);
		return;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri);
	log("Finished parsing: ", uri);
	if (program == nullptr) {
		log("Failed to parse program: ", uri);
		return;
	}
	publishDiagnostics(program);
	program_pool.open_file(uri); // Mark the file as open
	return;
}
