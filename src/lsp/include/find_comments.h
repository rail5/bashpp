/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <string>

#include <lsp/ProgramPool.h>
#include <IR/bpp.h>

/**
 * @brief Finds comments ostensibly associated with the given entity's definition.
 * 
 * This function searches for contiguous comment blocks that are located immediately before the entity's definition position.
 * It returns a string containing the comments, or an empty string if no comments are found.
 * 
 * @param entity The entity to find comments for.
 * @param program_pool The program pool to use for accessing source files.
 * @return A string containing the comments associated with the entity, or an empty string if no comments are found.
 */
std::string find_comments_for_entity(const bpp::IR::Entity* entity, ProgramPool* program_pool);
