/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_INTERNAL_ERROR_H_
#define SRC_INTERNAL_ERROR_H_

#include <stdexcept>
#include <antlr4-runtime.h>

/**
 * @struct internal_error
 * 
 * @brief An exception thrown when an internal error occurs
 * 
 * Internal errors are errors which should never occur in normal operation.
 * They are indicative of a bug in the Bash++ compiler, and halt compilation.
 */
struct internal_error : public std::runtime_error {
	internal_error(const std::string& msg, antlr4::tree::ParseTree* location);
	explicit internal_error(const std::string& msg);
};

#endif // SRC_INTERNAL_ERROR_H_
