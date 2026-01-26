/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DocumentSymbolRequest.h>

GenericResponseMessage bpp::BashppServer::handleDocumentSymbol(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	DocumentSymbolRequestResponse response;
	response.id = request.id;
	DocumentSymbolRequest document_symbol_request = request.toSpecific<DocumentSymbolParams>();

	DocumentSymbol symbol;
	symbol.name = "ExampleClass";
	symbol.kind = SymbolKind::Class;
	symbol.range.start.line = 0;
	symbol.range.start.character = 0;
	symbol.range.end.line = 0;
	symbol.range.end.character = 10;
	symbol.selectionRange.start.line = 0;
	symbol.selectionRange.start.character = 0;
	symbol.selectionRange.end.line = 0;
	symbol.selectionRange.end.character = 10;

	response.result = std::vector<DocumentSymbol>{symbol};

	return response;
}
