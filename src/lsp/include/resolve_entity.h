/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */
#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <cstdint>

#include "../../AST/ASTNode.h"
#include "../../bpp_include/bpp_codegen.h"
#include "explode.h"

std::shared_ptr<AST::ASTNode> find_node_at_position(std::shared_ptr<AST::ASTNode> node, uint32_t line, uint32_t column);

/**
 * @brief Resolves the entity referenced at the given line and column in the specified file.
 * 
 * This function uses the parser to parse the line content and find the entity which is referenced at the specified column.
 * It returns a shared pointer to the resolved entity, or nullptr if no entity is found.
 * 
 * E.g, if the segment pointed to by the position contains `@MyClass myObject`, we will return a pointer to the `MyClass` class.
 * If the segment contains `@myObject.myMethod`, we will return a pointer to the `myMethod` method of the `myObject` object.
 * If the segment contains `@myObject`, we will return a pointer to the `myObject` object.
 * If the segment contains `@include <file>`, we will return a newly-constructed faux-entity that points to the resolved path of the included file.
 * 
 * @param file The file to resolve the entity in.
 * @param line The line number to resolve the entity at (0-indexed).
 * @param column The column number to resolve the entity at (0-indexed).
 * @param program The program to resolve the entity in.
 * @param utf16_mode Whether to use UTF-16 mode for character counting.
 * @param file_contents Alternative contents of the file to use instead of reading from the filesystem.
 * @return A shared pointer to the resolved entity, or nullptr if no entity is found.
 */
std::shared_ptr<bpp::bpp_entity> resolve_entity_at(
	const std::string& file,
	uint32_t line,
	uint32_t column,
	std::shared_ptr<bpp::bpp_program> program,
	bool utf16_mode,
	const std::string& file_contents = ""
);

/**
 * @brief Finds comments ostensibly associated with the given entity's definition.
 * 
 * This function searches for contiguous comment blocks that are located immediately before the entity's definition position.
 * It returns a string containing the comments, or an empty string if no comments are found.
 * 
 * @param entity The entity to find comments for.
 * @return A string containing the comments associated with the entity, or an empty string if no comments are found.
 */
std::string find_comments_for_entity(std::shared_ptr<bpp::bpp_entity> entity);
