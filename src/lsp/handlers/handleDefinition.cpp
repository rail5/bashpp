/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DefinitionRequest.h>
#include <lsp/generated/ErrorCodes.h>
#include <lsp/include/resolve_entity.h>
#include <lsp/include/validateUri.h>

GenericResponseMessage bpp::BashppServer::handleDefinition(const GenericRequestMessage& request) {
	DefinitionRequest definition_request = request.toSpecific<DefinitionParams>();
	DefinitionRequestResponse response;
	response.id = request.id;

	std::string uri = definition_request.params.textDocument.uri;

	try {
		uri = validateUri(definition_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in definition request: ", e.what());
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "Invalid URI: " + definition_request.params.textDocument.uri;
		return response;
	}

	Position position = definition_request.params.position;
	log("Received Goto Definition request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri);

	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.error.code = static_cast<int>(ErrorCodes::InternalError);
		response.error.message = "Program not found for URI: " + uri;
		return response;
	}

	std::shared_ptr<bpp::bpp_entity> referenced_entity = nullptr;
	
	try {
		referenced_entity = resolve_entity_at(
			uri,
			position.line,
			position.character,
			program
		);
	} catch (...) {
		// Ignore, it'll just be nullptr.
	}

	if (referenced_entity == nullptr) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	bpp::SymbolPosition definition_location = referenced_entity->get_initial_definition();
	
	// Verify that a definition location was found
	if (definition_location.file.empty()) {
		log("No definition found for entity: ", referenced_entity->get_name(), " at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	Location location;
	location.uri = "file://" + definition_location.file;
	location.range.start.line = definition_location.line ;
	location.range.start.character = definition_location.column;
	location.range.end.line = definition_location.line;
	location.range.end.character = definition_location.column + referenced_entity->get_name().size();

	response.result = std::vector<Location>{location};
	log("Found definition for entity: ", referenced_entity->get_name(), " at URI: ", uri, ", Range: (", location.range.start.line, ", ", location.range.start.character, ") to (", location.range.end.line, ", ", location.range.end.character, ")");
	return response;
}
