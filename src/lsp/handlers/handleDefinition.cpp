/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/DefinitionRequest.h"


GenericResponseMessage bpp::BashppServer::handleDefinition(const GenericRequestMessage& request) {
	DefinitionRequest definition_request = request.toSpecific<DefinitionParams>();
	DefinitionRequestResponse response;
	response.id = request.id;

	std::string uri = definition_request.params.textDocument.uri;
	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to go to definition for non-local file: ", uri);
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "Invalid URI: " + uri;
		return response;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
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

	bpp::entity_reference referenced_entity = bpp::resolve_reference_at(uri, position.line, position.character, program);

	if (referenced_entity.entity == nullptr) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	bpp::SymbolPosition definition_location = referenced_entity.entity->get_initial_definition();

	Location location;
	location.uri = "file://" + definition_location.file;
	location.range.start.line = definition_location.line;
	location.range.start.character = definition_location.column;
	location.range.end.line = definition_location.line;
	location.range.end.character = definition_location.column + referenced_entity.entity->get_name().size();

	response.result = std::vector<Location>{location};
	log("Found definition for entity: ", referenced_entity.entity->get_name(), " at URI: ", uri, ", Range: (", location.range.start.line, ", ", location.range.start.character, ") to (", location.range.end.line, ", ", location.range.end.character, ")");
	return response;
}
