/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_INTERNAL_ERROR_CPP_
#define SRC_INTERNAL_ERROR_CPP_

#include <stdexcept>

/**
 * @struct internal_error
 * 
 * @brief An exception thrown when an internal error occurs
 * 
 * Internal errors are errors which should never occur in normal operation.
 * They are indicative of a bug in the Bash++ compiler, and halt compilation.
 */
struct internal_error : public std::runtime_error {
	internal_error(const std::string& msg, antlr4::tree::ParseTree* location)
		: std::runtime_error(msg + "\nYou've found a bug! Please report it.\nContext: " + location->getText()) {}
	
	explicit internal_error(const std::string& msg) : std::runtime_error(msg + "\nYou've found a bug! Please report it.") {}
};

#endif // SRC_INTERNAL_ERROR_CPP_
