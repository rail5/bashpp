/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <string>
#include <vector>
#include <AST/Position.h>

namespace AST {
struct ParserError {
	std::string message;
	AST::FilePosition start;
	AST::FilePosition end;
};
} // namespace AST
