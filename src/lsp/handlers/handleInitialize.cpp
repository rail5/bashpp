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
	result.capabilities.textDocumentSync = static_cast<TextDocumentSyncKind>(1); // Full sync mode
	result.capabilities.hoverProvider = true;

	CompletionOptions completionProvider;
	result.capabilities.completionProvider = completionProvider;
	result.capabilities.completionProvider->resolveProvider = false;
	result.capabilities.completionProvider->triggerCharacters = {".", "@"};

	result.capabilities.definitionProvider = true;
	result.capabilities.renameProvider = true;
	result.capabilities.documentSymbolProvider = true;
	result.capabilities.workspaceSymbolProvider = true;

	response.result = result;

	return response;
}
