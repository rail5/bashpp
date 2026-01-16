/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <cstdint>

namespace AST {

struct FilePosition {
	uint32_t line = 0;
	uint32_t column = 0;

	operator uint64_t() const {
		return (static_cast<uint64_t>(line) << 32) | column;
	}
};

} // namespace AST
