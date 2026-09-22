/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>

namespace bpp {

/**
 * @brief Used to label AST nodes and Entity tree nodes that are possible exit points for the containing control flow.
 *
 *  - 'exit' will exit the entire program (PROGRAM_EXIT)
 *  - 'exec', under certain conditions, will de-facto exit the entire program (PROGRAM_EXIT)
 *  - 'return' will exit the current function/method (FUNCTION_EXIT)
 * Loop exits ('break' and 'continue') are handled via different AST nodes / entity types, so not included here
 */
enum class ExitPointType : std::uint8_t {
	NO_EXIT = 0,
	FUNCTION_EXIT, // 'return'
	PROGRAM_EXIT, // 'exit' or 'exec'
};

} // namespace bpp
