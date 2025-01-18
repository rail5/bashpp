/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_INTERNAL_ERROR_CPP_
#define ANTLR_INTERNAL_ERROR_CPP_

#include <stdexcept>

struct internal_error : public std::runtime_error {
	explicit internal_error(const std::string& msg)
		: std::runtime_error(msg + "\nYou've found a bug! Please report it.") {}
};

#endif // ANTLR_INTERNAL_ERROR_CPP_