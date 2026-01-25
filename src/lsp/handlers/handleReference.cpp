/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/ReferencesRequest.h"
#include "../include/resolve_entity.h"


GenericResponseMessage bpp::BashppServer::handleReferences(const GenericRequestMessage& request) {
	ReferencesRequestResponse response;
	response.id = request.id;
	ReferencesRequest reference_request = request.toSpecific<ReferenceParams>();
	std::string uri = reference_request.params.textDocument.uri;

	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to find references for non-local file: ", uri);
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "Invalid URI: " + uri;
		return response;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_entity> entity = resolve_entity_at(
		uri,
		reference_request.params.position.line,
		reference_request.params.position.character,
		program_pool.get_program(uri),
		program_pool.get_utf16_mode(),
		unsaved_changes.find(uri) != unsaved_changes.end() ? unsaved_changes[uri] : "" // Send unsaved changes content if available
	);

	if (!entity) {
		// No entity to find references for
		log("No entity found at position: (", reference_request.params.position.line, ", ", reference_request.params.position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::vector<Location> locations;
	for (const auto& pos : entity->get_references()) {
		log("Found reference in file: ", pos.file, " at line: ", pos.line, " character: ", pos.column);
		std::string file_uri = "file://" + pos.file;
		Location loc;
		loc.uri = file_uri;
		loc.range.start.line = pos.line;
		loc.range.start.character = pos.column;
		loc.range.end.line = pos.line;
		loc.range.end.character = pos.column + entity->get_name().length();
		locations.push_back(loc);
	}

	response.result = locations;
	return response;
}
