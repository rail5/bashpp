/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"

#include "../generated/InitializeRequest.h"
#include "../generated/InitializeResult.h"

GenericResponseMessage bpp::BashppServer::handleInitialize(const GenericRequestMessage& request) {
	InitializeRequest initialize_request = request.toSpecific<InitializeParams>();
	InitializeRequestResponse response;
	response.id = request.id;

	InitializeResult result;
	result.capabilities.textDocumentSync = TextDocumentSyncKind::Full; // Full sync mode

	// Advertise that we support hover requests
	result.capabilities.hoverProvider = true;

	// Advertise that we support completion requests
	CompletionOptions completionProvider;
	result.capabilities.completionProvider = completionProvider;
	result.capabilities.completionProvider->resolveProvider = false;
	result.capabilities.completionProvider->triggerCharacters = {".", "@"};

	// Advertise that we support definition requests
	result.capabilities.definitionProvider = true;

	// Advertise that we support rename requests
	result.capabilities.renameProvider = true;

	// Advertise that we support references requests
	result.capabilities.referencesProvider = true;

	// Planned but not yet implemented:
	//result.capabilities.documentSymbolProvider = true;
	//result.capabilities.workspaceSymbolProvider = true;

	// If the client advertises that it supports UTF-8 position data, respond to let it know that's what we'll be sending
	if (initialize_request.params.capabilities.general.has_value()
		&& initialize_request.params.capabilities.general->positionEncodings.has_value()
		&& std::find(
				initialize_request.params.capabilities.general->positionEncodings->begin(),
				initialize_request.params.capabilities.general->positionEncodings->end(),
				PositionEncodingKind::UTF8)
			!= initialize_request.params.capabilities.general->positionEncodings->end()) {
		result.capabilities.positionEncoding = PositionEncodingKind::UTF8;
	} else {
		program_pool.set_utf16_mode(true);
		result.capabilities.positionEncoding = PositionEncodingKind::UTF16;
	}

	response.result = result;

	return response;
}
