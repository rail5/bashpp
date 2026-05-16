/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DocumentSymbolRequest.h>
#include <lsp/include/validateUri.h>

GenericResponseMessage bpp::BashppServer::handleDocumentSymbol(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	DocumentSymbolRequestResponse response;
	response.id = request.id;
	DocumentSymbolRequest document_symbol_request = request.toSpecific<DocumentSymbolParams>();
	std::string uri;

	try {
		uri = validateUri(document_symbol_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in Document Symbol request: ", e.what());
		response.result = nullptr;
		return response;
	}

	auto program = program_pool.get_program(uri);
	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::vector<DocumentSymbol> result;

	// First, populate with all the classes whose definition positions are within the requested document
	const auto& classes = program->get_all_known_classes();
	for (const auto& cls : classes) {
		if (cls == nullptr) continue;
		if (cls->get_initial_definition().file != uri) continue;
		if (cls->get_name() == "primitive") continue; // The primitive class is an implementation detail that we don't want to expose in the language server

		DocumentSymbol symbol;
		symbol.name = cls->get_name();
		symbol.kind = SymbolKind::Class;
		symbol.range.start.line = cls->get_initial_definition().line;
		symbol.range.start.character = cls->get_initial_definition().column;
		symbol.range.end.line = cls->get_initial_definition().line;
		symbol.range.end.character = cls->get_initial_definition().column + cls->get_name().length();
		symbol.selectionRange = symbol.range; // For simplicity, selection range is the same as the full range for now

		// Get all the methods and data members defined within the class and add them as children of the class symbol
		std::vector<DocumentSymbol> children;
		for (const auto& method : cls->get_methods()) {
			if (method->get_name().starts_with("__")) continue; // Skip system methods like __new and __delete
			if (method->get_initial_definition().file != uri) continue; // Only include methods defined in this document, not those inherited from other classes
			DocumentSymbol method_symbol;
			method_symbol.name = method->get_name();
			method_symbol.kind = SymbolKind::Method;
			method_symbol.range.start.line = method->get_initial_definition().line;
			method_symbol.range.start.character = method->get_initial_definition().column;
			method_symbol.range.end.line = method->get_initial_definition().line;
			method_symbol.range.end.character = method->get_initial_definition().column + method->get_name().length();
			method_symbol.selectionRange = method_symbol.range; // For simplicity, selection range is the same as the full range for now
			children.push_back(method_symbol);
		}
		for (const auto& datamember : cls->get_datamembers()) {
			if (datamember->get_initial_definition().file != uri) continue; // Only include data members defined in this document, not those inherited from other classes
			DocumentSymbol datamember_symbol;
			datamember_symbol.name = datamember->get_name();
			datamember_symbol.kind = SymbolKind::Property;
			datamember_symbol.range.start.line = datamember->get_initial_definition().line;
			datamember_symbol.range.start.character = datamember->get_initial_definition().column;
			datamember_symbol.range.end.line = datamember->get_initial_definition().line;
			datamember_symbol.range.end.character = datamember->get_initial_definition().column + datamember->get_name().length();
			datamember_symbol.selectionRange = datamember_symbol.range; // For simplicity, selection range is the same as the full range for now
			children.push_back(datamember_symbol);
		}

		symbol.children = children;

		result.push_back(symbol);
	}

	// Next, we'll get all the objects in the document
	//
	// I don't know whether the conventional response would include
	// "internal" document symbols (e.g., objects instantiated inside
	// some function somewhere, not visible to the outside world)
	// If so, we can amend this to call get_all_known_objects() on
	// each entity in the program's entity map and concatenate the results
	//
	// But it seems more reasonable to me that a "DocumentSymbolRequest"
	// would anticipate only symbols that are "exported" from this document
	// to others. I.e., those that would be available after @include <ThisSource.bpp>

	for (const auto& object : program->get_all_known_objects()) {
		if (object == nullptr) continue;
		if (object->get_initial_definition().file != uri) continue;
		DocumentSymbol symbol;
		symbol.name = object->get_name();
		symbol.kind = SymbolKind::Object;
		symbol.range.start.line = object->get_initial_definition().line;
		symbol.range.start.character = object->get_initial_definition().column;
		symbol.range.end.line = object->get_initial_definition().line;
		symbol.range.end.character = object->get_initial_definition().column + object->get_name().length();
		symbol.selectionRange = symbol.range; // For simplicity, selection range is the same as the full range for now

		result.push_back(symbol);
	}

	response.result = result;
	return response;
}
