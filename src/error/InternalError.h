/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <source_location>
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

	InternalError(const std::string& msg, std::source_location location)
		: std::runtime_error(msg + "\nYou've found a bug! Please report it.\nAt " + location.file_name() + ":" + std::to_string(location.line())) {}
};

} // namespace bpp::ErrorHandling

#ifndef NDEBUG
	#define bpp_assert(expr, msg) \
		do { \
			if (!(expr)) { \
				throw bpp::ErrorHandling::InternalError(msg, std::source_location::current()); \
			} \
		} while (false)
#else
	#define bpp_assert(expr, msg) ((void)0)
#endif
