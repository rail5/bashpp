/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/RenameRequest.h"

GenericResponseMessage bpp::BashppServer::handleRename(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	RenameRequestResponse response;
	response.id = request.id;
	RenameRequest rename_request = request.toSpecific<RenameParams>();
	std::string uri = rename_request.params.textDocument.uri;
	Position position = rename_request.params.position;
	std::string new_name = rename_request.params.newName;

	WorkspaceEdit edit;
	TextEdit text_edit;
	text_edit.range.start.line = position.line;
	text_edit.range.start.character = position.character;
	text_edit.range.end.line = position.line;
	text_edit.range.end.character = position.character + static_cast<uint32_t>(new_name.size());
	text_edit.newText = new_name;

	if (!edit.changes.has_value()) {
		edit.changes = std::unordered_map<std::string, std::vector<TextEdit>>{};
	}
	edit.changes->operator[](uri).push_back(text_edit);

	response.result = edit;

	return response;
}
