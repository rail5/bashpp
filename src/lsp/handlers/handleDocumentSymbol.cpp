/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DocumentSymbolRequest.h>
#include <lsp/include/validateUri.h>

#include <IR/entities/Program.h>
#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Object.h>

GenericResponseMessage bpp::LSP::BashppServer::handleDocumentSymbol(const GenericRequestMessage& request) {
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

	auto program = program_pool.getProgram(uri);
	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::vector<DocumentSymbol> result;

	// First, populate with all the classes whose definition positions are within the requested document
	const auto& classes = program->program->getAllKnownClasses();
	for (const auto& cls : classes) {
		if (cls == nullptr) continue;
		auto class_definition_position = program->source_index->getDefinitionPosition(cls);
		if (!class_definition_position || class_definition_position->source_file != uri) continue;

		DocumentSymbol symbol;
		symbol.name = cls->getName();
		symbol.kind = SymbolKind::Class;
		symbol.range.start.line = class_definition_position->start.line;
		symbol.range.start.character = class_definition_position->start.column;
		symbol.range.end.line = class_definition_position->end.line;
		symbol.range.end.character = class_definition_position->end.column;
		symbol.selectionRange = symbol.range; // For simplicity, selection range is the same as the full range for now

		// Get all the methods and data members defined within the class and add them as children of the class symbol
		std::vector<DocumentSymbol> children;
		for (const auto& method : cls->getAllMethods()) {
			if (method->getName().starts_with("__")) continue; // Skip system methods like __new and __delete
			auto method_definition_position = program->source_index->getDefinitionPosition(method.get());
			// Only include methods defined in this document, not those inherited from other classes
			if (!method_definition_position || method_definition_position->source_file != uri) continue;
			DocumentSymbol method_symbol;
			method_symbol.name = method->getName();
			method_symbol.kind = SymbolKind::Method;
			method_symbol.range.start.line = method_definition_position->start.line;
			method_symbol.range.start.character = method_definition_position->start.column;
			method_symbol.range.end.line = method_definition_position->end.line;
			method_symbol.range.end.character = method_definition_position->end.column;
			method_symbol.selectionRange = method_symbol.range; // For simplicity, selection range is the same as the full range for now
			children.push_back(method_symbol);
		}
		for (const auto& datamember : cls->getAllDatamembers()) {
			auto datamember_definition_position = program->source_index->getDefinitionPosition(datamember.get());
			// Only include data members defined in this document, not those inherited from other classes
			if (!datamember_definition_position || datamember_definition_position->source_file != uri) continue;
			DocumentSymbol datamember_symbol;
			datamember_symbol.name = datamember->getName();
			datamember_symbol.kind = SymbolKind::Property;
			datamember_symbol.range.start.line = datamember_definition_position->start.line;
			datamember_symbol.range.start.character = datamember_definition_position->start.column;
			datamember_symbol.range.end.line = datamember_definition_position->end.line;
			datamember_symbol.range.end.character = datamember_definition_position->end.column;
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

	for (const auto* object : program->program->getAllKnownObjects()) {
		if (object == nullptr) continue;
		auto object_definition_position = program->source_index->getDefinitionPosition(object);
		if (!object_definition_position || object_definition_position->source_file != uri) continue;
		DocumentSymbol symbol;
		symbol.name = object->getName();
		symbol.kind = SymbolKind::Object;
		symbol.range.start.line = object_definition_position->start.line;
		symbol.range.start.character = object_definition_position->start.column;
		symbol.range.end.line = object_definition_position->end.line;
		symbol.range.end.character = object_definition_position->end.column;
		symbol.selectionRange = symbol.range; // For simplicity, selection range is the same as the full range for now

		result.push_back(symbol);
	}

	response.result = result;
	return response;
}
