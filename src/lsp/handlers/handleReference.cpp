/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/ReferencesRequest.h>
#include <lsp/generated/ErrorCodes.h>
#include <lsp/include/resolve_entity.h>
#include <lsp/include/validateUri.h>

#include <bpp_include/bpp_entity.h>

GenericResponseMessage bpp::BashppServer::handleReferences(const GenericRequestMessage& request) {
	ReferencesRequestResponse response;
	response.id = request.id;
	ReferencesRequest reference_request = request.toSpecific<ReferenceParams>();
	std::string uri = reference_request.params.textDocument.uri;

	try {
		uri = validateUri(reference_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in References request: ", e.what());
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InvalidParams),
			"Invalid URI: " + reference_request.params.textDocument.uri, nullptr};
		return response;
	}

	auto entities = find_all_entities_for(
		uri,
		reference_request.params.position.line,
		reference_request.params.position.character,
		program_pool.get_programs_for_file(uri)
	);

	if (entities.empty()) {
		// No entity to find references for
		log("No entity found at position: (", reference_request.params.position.line, ", ", reference_request.params.position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::vector<Location> locations;
	for (const auto& entity : entities) {
		for (const auto& pos : entity->get_references()) {
			log("Found reference in file: ", pos.file, " at line: ", pos.line, " character: ", pos.column);
			std::string file_uri = "file://" + pos.file;
			Location loc;
			loc.uri = file_uri;
			loc.range.start.line = pos.line;
			loc.range.start.character = pos.column;
			loc.range.end.line = pos.line;
			loc.range.end.character = pos.column + entity->get_name().length();

			// Make sure we don't add duplicate locations
			// (e.g., in the event that multiple programs
			// include this source file, we may find the
			// same reference multiple times)
			bool already_added = std::ranges::any_of(locations, [&](const Location& existing_loc) {
				return existing_loc.uri == loc.uri
					&& existing_loc.range.start.line == loc.range.start.line
					&& existing_loc.range.start.character == loc.range.start.character
					&& existing_loc.range.end.line == loc.range.end.line
					&& existing_loc.range.end.character == loc.range.end.character;
			});

			if (!already_added) {
				locations.push_back(loc);
			}
		}
	}

	response.result = locations;
	return response;
}
