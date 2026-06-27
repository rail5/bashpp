/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <cstdint>

namespace bpp::AST {

enum class AccessModifier : std::uint8_t {
	PUBLIC,
	PROTECTED,
	PRIVATE
};

} // namespace bpp::AST
