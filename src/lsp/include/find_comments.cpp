/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "find_comments.h"

#include <IR/entities/Entity.h>

#include <sstream>
#include <string>
#include <cstdint>

namespace bpp::LSP {

std::string find_comments_for_entity(const bpp::IR::Entity* entity, bpp::LSP::ProgramPool* program_pool) {
	if (entity == nullptr) return "";

	// Locate the entity's definition position
	auto containing_program = program_pool->getProgramForEntity(entity);
	if (!containing_program) return ""; // No containing program found, no comments can be found
	auto definition_position = containing_program->source_index->getDefinitionPosition(entity);
	if (!definition_position) return ""; // No definition position found, no comments can be found

	std::istringstream source_file_contents(program_pool->getFileContents(std::filesystem::path(definition_position->source_file)));

	std::string comments;
	std::string line;
	std::uint32_t current_line = 0;
	while (std::getline(source_file_contents, line)) {
		if (current_line >= definition_position->start.line) break; // Stop reading after reaching the definition line
		current_line++;

		std::size_t comment_start = line.find('#');
		if (comment_start != std::string::npos) {
			// Extract the comment part
			std::string comment = line.substr(comment_start);
			// Trim leading whitespace from the comment
			comment.erase(0, comment.find_first_not_of(" \t"));

			if (!comments.empty()) comments += "\n"; // Separate comments with a newline
			comments += comment; // Append this comment to the comment block
		} else {
			comments.clear(); // A comment block ended, and we still haven't hit the definition line
				// In other words: the previous comment block is not associated with the entity
				// Therefore, clear it
		}
	}
	return comments;
}

}
