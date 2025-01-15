/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_SYNTAX_ERROR_CPP_
#define ANTLR_SYNTAX_ERROR_CPP_

#include <stdexcept>

struct syntax_error : public std::runtime_error {
	syntax_error(const std::string& msg,
		std::string source_file,
		int line_number)
		: std::runtime_error(source_file + ":" + std::to_string(line_number) + ": " + msg) {}
	
	syntax_error(const std::string& msg,
		std::string source_file,
		int line_number,
		int column_number)
		: std::runtime_error(source_file + ":" + std::to_string(line_number) + ":" + std::to_string(column_number) + ": " + msg) {}
};

#endif // ANTLR_SYNTAX_ERROR_CPP_