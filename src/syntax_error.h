/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_SYNTAX_ERROR_H_
#define SRC_SYNTAX_ERROR_H_

#include <string>
#include <stack>

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
	int line,
	int column,
	const std::string& text,
	const std::string& msg,
	std::stack<std::string> include_chain,
	bool is_warning = false);

#endif // SRC_SYNTAX_ERROR_H_
