/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once
#include <cstdint>

namespace AST {

enum class AccessModifier : uint8_t {
	PUBLIC,
	PROTECTED,
	PRIVATE
};

} // namespace AST
