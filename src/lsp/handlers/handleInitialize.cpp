/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>

#include <lsp/generated/InitializeRequest.h>
#include <lsp/generated/InitializeResult.h>

GenericResponseMessage bpp::BashppServer::handleInitialize(const GenericRequestMessage& request) {
	InitializeRequest initialize_request = request.toSpecific<InitializeParams>();
	InitializeRequestResponse response;
	response.id = request.id;

	InitializeResult result;
	result.capabilities.textDocumentSync = TextDocumentSyncKind::Full; // Full sync mode

	// Advertise that we support hover requests
	result.capabilities.hoverProvider = true;

	// Advertise that we support completion requests
	result.capabilities.completionProvider = CompletionOptions{
		.triggerCharacters = std::vector<std::string>{".", "@"},
		.resolveProvider = false
	};

	// Advertise that we support definition requests
	result.capabilities.definitionProvider = true;

	// Advertise that we support rename requests
	result.capabilities.renameProvider = true;

	// Advertise that we support references requests
	result.capabilities.referencesProvider = true;

	// Advertise that we support DocumentSymbol requests
	result.capabilities.documentSymbolProvider = true;

	// Planned but not yet implemented:
	//result.capabilities.workspaceSymbolProvider = true;

	// If the client advertises that it supports UTF-8 position data, respond to let it know that's what we'll be sending
	if (initialize_request.params.capabilities.general.has_value()
		&& initialize_request.params.capabilities.general->positionEncodings.has_value()
		&& std::ranges::contains(*initialize_request.params.capabilities.general->positionEncodings, PositionEncodingKind::UTF8)
	) {
		result.capabilities.positionEncoding = PositionEncodingKind::UTF8;
	} else {
		program_pool.set_utf16_mode(true);
		result.capabilities.positionEncoding = PositionEncodingKind::UTF16;
	}

	response.result = result;

	return response;
}
