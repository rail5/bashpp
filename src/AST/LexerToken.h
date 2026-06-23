/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

#include <include/ParserPosition.h>

namespace AST {

/**
 * @brief A lexer symbol and its exact source range for editor features.
 */
struct LexerToken {
	std::string kind;
	std::string text;
	ParserLocation location;
};

} // namespace AST
