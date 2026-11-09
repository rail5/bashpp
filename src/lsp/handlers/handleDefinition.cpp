/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/DefinitionRequest.h>
#include <lsp/generated/ErrorCodes.h>
#include <lsp/include/validateUri.h>

#include <IR/entities/Entity.h>
#include <IR/entities/components/Named.h>

GenericResponseMessage bpp::BashppServer::handleDefinition(const GenericRequestMessage& request) {
	DefinitionRequest definition_request = request.toSpecific<DefinitionParams>();
	DefinitionRequestResponse response;
	response.id = request.id;

	std::string uri = definition_request.params.textDocument.uri;

	try {
		uri = validateUri(definition_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in definition request: ", e.what());
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InvalidParams),
			"Invalid URI: " + definition_request.params.textDocument.uri, nullptr};
		return response;
	}

	Position position = definition_request.params.position;
	log("Received Goto Definition request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	auto program = program_pool.getProgram(uri);

	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InternalError),
			"Program not found for URI: " + uri, nullptr};
		return response;
	}

	const auto* referenced_entity = program->source_index->getReferencedEntity({uri, position.line, position.character});

	if (referenced_entity == nullptr) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::string entity_name = "(unnamed)";
	if (const auto* named_entity = dynamic_cast<const bpp::IR::Components::Named*>(referenced_entity)) {
		entity_name = named_entity->getName();
	}

	auto definition_location = program->source_index->getDefinitionPosition(referenced_entity);

	// Verify that a definition location was found
	if (!definition_location) {
		log("No definition found for entity: ", entity_name, " at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	Location location;
	location.uri = "file://" + definition_location->source_file.string();
	location.range.start.line = definition_location->start.line;
	location.range.start.character = definition_location->start.column;
	location.range.end.line = definition_location->end.line;
	location.range.end.character = definition_location->end.column;

	response.result = std::vector<Location>{location};
	log("Found definition for entity: ", entity_name, " at URI: ", uri, ", Range: (", location.range.start.line, ", ", location.range.start.character, ") to (", location.range.end.line, ", ", location.range.end.character, ")");
	return response;
}
