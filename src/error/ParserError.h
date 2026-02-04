/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
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
