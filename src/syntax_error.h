/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_SYNTAX_ERROR_H_
#define SRC_SYNTAX_ERROR_H_

#include <string>
#include <stack>
#include <memory>
#include <cstdint>
#include "bpp_include/bpp.h"

/**
 * @brief Print a syntax error or warning message to stderr
 * @param source_file The source file which contains the error
 * @param line The line number where the error occurred
 * @param column The column number where the error occurred
 * @param text The text of the token which caused the error
 * @param msg The error message to display
 * @param include_chain A stack of include files which led to the error
 * @param is_warning Whether the message is a warning or an error
 */
void print_syntax_error_or_warning(
	std::string source_file,
	uint32_t line,
	uint32_t column,
	const std::string& text,
	const std::string& msg,
	std::stack<std::string> include_chain,
	std::shared_ptr<bpp::bpp_program> program,
	bool is_warning = false);

void print_syntax_error_or_warning(
	std::string source_file,
	uint32_t line,
	uint32_t column,
	uint32_t error_portion_length,
	const std::string& msg,
	std::stack<std::string> include_chain,
	std::shared_ptr<bpp::bpp_program> program,
	bool is_warning = false);

void print_syntax_error_from_parser(
	std::string source_file,
	uint32_t line,
	uint32_t start_column,
	uint32_t end_column,
	const std::string& msg,
	std::stack<std::string> include_chain
);

// Helper functions
// Should probably be moved to a separate file or somehow better organized
std::string utf8_substr(const std::string& str, uint32_t start, uint32_t length);
uint32_t utf8_length(const std::string& str);
std::string equal_width_padding(const std::string& str, char padding_char = ' ');

#endif // SRC_SYNTAX_ERROR_H_
