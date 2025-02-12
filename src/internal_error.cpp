/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_INTERNAL_ERROR_CPP_
#define SRC_INTERNAL_ERROR_CPP_

#include <stdexcept>

struct internal_error : public std::runtime_error {
	explicit internal_error(const std::string& msg)
		: std::runtime_error(msg + "\nYou've found a bug! Please report it.") {}
};

#endif // SRC_INTERNAL_ERROR_CPP_
