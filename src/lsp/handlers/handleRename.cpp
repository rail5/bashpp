/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/RenameRequest.h>
#include <lsp/include/resolve_entity.h>

GenericResponseMessage bpp::BashppServer::handleRename(const GenericRequestMessage& request) {
	RenameRequestResponse response;
	response.id = request.id;
	RenameRequest rename_request = request.toSpecific<RenameParams>();
	std::string uri = rename_request.params.textDocument.uri;
	
	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to handle rename for non-local file: ", uri);
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "Invalid URI: " + uri;
		return response;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_entity> entity = resolve_entity_at(
		uri,
		rename_request.params.position.line,
		rename_request.params.position.character,
		program_pool.get_program(uri)
	);

	// Verify that we found an entity, and it is either:
	// 1. An object
	// 2. A class
	// 3. A method
	// 4. A data member (covered by 'object' case)
	std::shared_ptr<bpp::bpp_object> obj = std::dynamic_pointer_cast<bpp::bpp_object>(entity);
	std::shared_ptr<bpp::bpp_class> cls = std::dynamic_pointer_cast<bpp::bpp_class>(entity);
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(entity);
	if (obj == nullptr && cls == nullptr && method == nullptr) {
		log("No valid entity found to rename at position: (", rename_request.params.position.line, ", ", rename_request.params.position.character, ") in URI: ", uri);
		// The spec says we should return an error here
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "No entity found to rename";
		response.result = nullptr;
		return response;
	}

	// Get a list of everywhere the entity is referenced
	std::list<bpp::SymbolPosition> references = entity->get_references();

	WorkspaceEdit edit;
	std::unordered_map<std::string, std::vector<TextEdit>> changes;

	std::string new_name = rename_request.params.newName;
	
	// Basic validation: Does the name satisfy the Bash++ naming rules?
	if (!bpp::is_valid_identifier(new_name)) {
		log("Invalid new name for entity: ", new_name);
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);

		if (new_name.find("__") != std::string::npos) {
			response.error.message = "Invalid new name: " + new_name + "\nBash++ identifiers cannot contain double underscores";
		} else {
			response.error.message = "Invalid new name: " + new_name;
		}

		response.result = nullptr;
		return response;
	}

	for (const auto& ref : references) {
		TextEdit single_rename;
		single_rename.range.start.line = ref.line;
		single_rename.range.start.character = ref.column;
		single_rename.range.end.line = ref.line;
		single_rename.range.end.character = ref.column + static_cast<uint32_t>(entity->get_name().size());
		single_rename.newText = new_name;

		changes["file://" + ref.file].push_back(single_rename);
	}

	if (changes.empty()) {
		log("No references found to rename for entity: ", entity->get_name());
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "No references found to rename";
		response.result = nullptr;
		return response;
	}

	edit.changes = changes;
	edit.documentChanges = std::nullopt; // Not using document changes for now

	if (rename_request.params.workDoneToken.has_value()) {
		edit.changeAnnotations = std::unordered_map<std::string, ChangeAnnotation>{};
		ChangeAnnotation annotation;
		annotation.label = "Renaming " + entity->get_name();
		edit.changeAnnotations->emplace("rename_annotation", annotation);
	}

	response.result = edit;
	return response;
}
