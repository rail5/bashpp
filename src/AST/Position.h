/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>

namespace bpp::AST {

struct FilePosition {
	std::uint32_t line = 0;
	std::uint32_t column = 0;

	operator std::uint64_t() const {
		return (static_cast<std::uint64_t>(line) << 32) | column;
	}
};

} // namespace bpp::AST
