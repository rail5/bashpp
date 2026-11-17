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
#include <IR/entities/Program.h>
#include <IR/entities/components/Named.h>
#include <IR/entities/components/Referenceable.h>

GenericResponseMessage bpp::LSP::BashppServer::handleDefinition(const GenericRequestMessage& request) {
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

	const auto& referenced_entities = program->getReferencedEntities({uri, position.line, position.character});

	if (referenced_entities.empty()) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	// FIXME(@rail5): This relies on insertion ordering
	// It assumes e.g. for method references that the most-derived implementation of the method will be the last entity in the set
	// Better ways to provide this guarantee?
	const auto* referenced_entity = referenced_entities.back();

	std::string entity_name = "(unnamed)";
	if (const auto* named_entity = dynamic_cast<const bpp::IR::Components::Named*>(referenced_entity)) {
		entity_name = named_entity->getName();
	}

	const auto* referenceable_entity = dynamic_cast<const bpp::IR::Components::Referenceable*>(referenced_entity);
	bpp_assert(referenceable_entity != nullptr, "Referenced entity is not referenceable");
	auto definition_location = referenceable_entity->getDefinitionPosition();

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
