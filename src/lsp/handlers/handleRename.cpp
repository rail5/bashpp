/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/RenameRequest.h>
#include <lsp/generated/ErrorCodes.h>
#include <lsp/include/validateUri.h>

#include <IR/bpp.h>
#include <IR/identifiers.h>
#include <IR/entities/Entity.h>
#include <IR/entities/components/Named.h>

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
	if (!bpp::IR::is_valid_identifier(new_name)) {
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

	auto entities = program_pool.findAllEntitiesForSymbol({uri, rename_request.params.position.line, rename_request.params.position.character});

	// Verify that we found an entity, and it has a name component (i.e., it is a Named entity)
	bool found_entities_to_rename = false;
	for (auto it = entities.begin(); it != entities.end(); ) {
		const auto* named_entity = dynamic_cast<const bpp::IR::Components::Named*>(*it);
		if (named_entity != nullptr) {
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

	std::string old_name = dynamic_cast<const bpp::IR::Components::Named*>(entities.front())->getName(); // Get the name of the first entity (they all have the same name)

	WorkspaceEdit edit;
	std::unordered_map<std::string, std::vector<TextEdit>> changes;

	for (const auto& entity : entities) {
		// Get a list of everywhere the entity is referenced
		//std::list<bpp::IR::SourceLocationRange> references = entity->get_references();
		auto containing_program = program_pool.getProgramForEntity(entity);
		if (!containing_program) {
			log("Entity found, but no containing program for entity: ", old_name);
			continue;
		}
		auto references = containing_program->source_index->getReferences(entity);

		for (const auto& ref : references) {
			TextEdit single_rename;
			single_rename.range.start.line = ref.start.line;
			single_rename.range.start.character = ref.start.column;
			single_rename.range.end.line = ref.end.line;
			single_rename.range.end.character = ref.end.column;
			single_rename.newText = new_name;

			// Make sure we don't add duplicate edits for the same file
			// (e.g., in the event that multiple programs include this source file, we may find the same reference multiple times)
			bool already_added = std::ranges::any_of(changes["file://" + ref.source_file.string()], [&](const TextEdit& edit) {
				return edit.range.start.line == single_rename.range.start.line
					&& edit.range.start.character == single_rename.range.start.character
					&& edit.range.end.line == single_rename.range.end.line
					&& edit.range.end.character == single_rename.range.end.character;
			});

			if (!already_added) {
				changes["file://" + ref.source_file.string()].push_back(single_rename);
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
