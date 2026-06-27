/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/RenameRequest.h>
#include <lsp/generated/ErrorCodes.h>
#include <lsp/include/resolve_entity.h>
#include <lsp/include/validateUri.h>

#include <entities/bpp.h>
#include <entities/bpp_entity.h>
#include <entities/bpp_object.h>
#include <entities/bpp_class.h>
#include <entities/bpp_method.h>

GenericResponseMessage bpp::BashppServer::handleRename(const GenericRequestMessage& request) {
	RenameRequestResponse response;
	response.id = request.id;
	RenameRequest rename_request = request.toSpecific<RenameParams>();
	std::string uri = rename_request.params.textDocument.uri;

	try {
		uri = validateUri(rename_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in Rename request: ", e.what());
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InvalidParams),
			"Invalid URI: " + rename_request.params.textDocument.uri, nullptr};
		return response;
	}

	std::string new_name = rename_request.params.newName;
	// Basic validation: Does the name satisfy Bash++ naming rules?
	if (!bpp::is_valid_identifier(new_name)) {
		log("Invalid new name for entity: ", new_name);
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InvalidParams),
			"Invalid new name: " + new_name, nullptr};

		if (new_name.contains("__")) {
			response.error->message += "\nBash++ identifiers cannot contain double underscores";
		}

		response.result = nullptr;
		return response;
	}

	auto entities = find_all_entities_for(
		uri,
		rename_request.params.position.line,
		rename_request.params.position.character,
		program_pool.get_programs_for_file(uri)
	);

	// Verify that we found an entity, and it is either:
	// 1. An object
	// 2. A class
	// 3. A method
	// 4. A data member (covered by 'object' case)
	// Also, remove any entities that are not valid for renaming (i.e., not of the above types)
	bool found_entities_to_rename = false;
	for (auto it = entities.begin(); it != entities.end(); ) {
		auto obj = std::dynamic_pointer_cast<bpp::bpp_object>(*it);
		auto cls = std::dynamic_pointer_cast<bpp::bpp_class>(*it);
		auto method = std::dynamic_pointer_cast<bpp::bpp_method>(*it);
		if (obj != nullptr || cls != nullptr || method != nullptr) {
			found_entities_to_rename = true;
			++it;
		} else {
			it = entities.erase(it); // Remove entities that are not valid for renaming
		}
	}

	if (!found_entities_to_rename) {
		log("No valid entity found to rename at position: (", rename_request.params.position.line, ", ", rename_request.params.position.character, ") in URI: ", uri);
		// The spec says we should return an error here
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InvalidParams),
			"No entity found to rename at position: ("
				+ std::to_string(rename_request.params.position.line) + ", "
				+ std::to_string(rename_request.params.position.character)
				+ ") in URI: " + uri, nullptr};
		response.result = nullptr;
		return response;
	}

	std::string old_name = entities.front()->get_name(); // Get the name of the first entity (they all have the same name)

	WorkspaceEdit edit;
	std::unordered_map<std::string, std::vector<TextEdit>> changes;

	for (const auto& entity : entities) {
		// Get a list of everywhere the entity is referenced
		std::list<bpp::SymbolPosition> references = entity->get_references();

		for (const auto& ref : references) {
			TextEdit single_rename;
			single_rename.range.start.line = ref.line;
			single_rename.range.start.character = ref.column;
			single_rename.range.end.line = ref.line;
			single_rename.range.end.character = ref.column + static_cast<std::uint32_t>(entity->get_name().size());
			single_rename.newText = new_name;

			// Make sure we don't add duplicate edits for the same file
			// (e.g., in the event that multiple programs include this source file, we may find the same reference multiple times)
			bool already_added = std::ranges::any_of(changes["file://" + ref.file], [&](const TextEdit& edit) {
				return edit.range.start.line == single_rename.range.start.line
					&& edit.range.start.character == single_rename.range.start.character
					&& edit.range.end.line == single_rename.range.end.line
					&& edit.range.end.character == single_rename.range.end.character;
			});

			if (!already_added) {
				changes["file://" + ref.file].push_back(single_rename);
			}
		}
	}

	if (changes.empty()) {
		log("No references found to rename for entity: ", old_name);
		response.error = ResponseError{
			static_cast<int>(ErrorCodes::InvalidParams),
			"No references found to rename for entity: " + old_name, nullptr};
		response.result = nullptr;
		return response;
	}

	edit.changes = changes;
	edit.documentChanges = std::nullopt; // Not using document changes for now

	if (rename_request.params.workDoneToken.has_value()) {
		edit.changeAnnotations = std::unordered_map<std::string, ChangeAnnotation>{};
		ChangeAnnotation annotation;
		annotation.label = "Renaming " + old_name;
		edit.changeAnnotations->emplace("rename_annotation", annotation);
	}

	response.result = edit;
	return response;
}
