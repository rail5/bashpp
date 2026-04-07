/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/ReferencesRequest.h>
#include <lsp/generated/ErrorCodes.h>
#include <lsp/include/resolve_entity.h>
#include <lsp/include/validateUri.h>

GenericResponseMessage bpp::BashppServer::handleReferences(const GenericRequestMessage& request) {
	ReferencesRequestResponse response;
	response.id = request.id;
	ReferencesRequest reference_request = request.toSpecific<ReferenceParams>();
	std::string uri = reference_request.params.textDocument.uri;

	try {
		uri = validateUri(reference_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in References request: ", e.what());
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "Invalid URI: " + reference_request.params.textDocument.uri;
		return response;
	}

	std::shared_ptr<bpp::bpp_entity> entity = resolve_entity_at(
		uri,
		reference_request.params.position.line,
		reference_request.params.position.character,
		program_pool.get_program(uri)
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
