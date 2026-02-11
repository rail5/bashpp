/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <stdexcept>

namespace bpp {
namespace ErrorHandling {

/**
 * @struct InternalError
 * 
 * @brief An exception thrown when an internal error occurs
 * 
 * Internal errors are errors which should never occur in normal operation.
 * They are indicative of a bug in the Bash++ compiler, and halt compilation.
 */
struct InternalError : public std::runtime_error {
	explicit InternalError(const std::string& msg)
		: std::runtime_error(msg + "\nYou've found a bug! Please report it.") {}
	
};

} // namespace ErrorHandling
} // namespace bpp
