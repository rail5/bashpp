/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <AST/Position.h>

namespace AST {
struct ParserError {
	std::string message;
	AST::FilePosition start;
	AST::FilePosition end;
};
} // namespace AST
