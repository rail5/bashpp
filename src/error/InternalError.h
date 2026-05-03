/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdexcept>

namespace bpp::ErrorHandling {

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
	
	InternalError(const std::string& msg, const std::string& file, int line)
		: std::runtime_error(msg + "\nYou've found a bug! Please report it.\nAt " + file + ":" + std::to_string(line)) {}
};

} // namespace bpp::ErrorHandling

#if !defined (NDEBUG)
	#define bpp_assert(expr, msg) \
		do { \
			if (!(expr)) { \
				throw bpp::ErrorHandling::InternalError(msg, __FILE__, __LINE__); \
			} \
		} while (false)
#else
	#define bpp_assert(expr, msg) ((void)0)
#endif
