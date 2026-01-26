/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <iostream>
#include <string>
#include <fstream>
#include <stack>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "syntax_error.h"

#include <utf8cpp/utf8.h>

void print_syntax_error_or_warning(
	std::string source_file,
	uint32_t line, uint32_t column, uint32_t text_length,
	const std::string& msg,
	std::stack<std::string> include_chain,
	std::shared_ptr<bpp::bpp_program> program,
	bool lsp_mode,
	bool is_warning
) {
	// Add to the program's diagnostics
	if (program != nullptr) {
		program->add_diagnostic(
			source_file,
			is_warning ? bpp::diagnostic_type::DIAGNOSTIC_WARNING : bpp::diagnostic_type::DIAGNOSTIC_ERROR,
			msg,
			line,
			column,
			line,
			column + text_length
		);
	}
	if (lsp_mode) return; // The language server doesn't need to print errors to stderr, just add them to the diagnostics list

	// Colorize output if the output is a TTY
	bool is_tty = isatty(fileno(stderr));
	std::string color_red = is_tty ? "\033[0;31m" : "";
	std::string color_purple = is_tty ? "\033[1;35m" : "";
	std::string color_orange = is_tty ? "\033[0;33m" : "";
	std::string color_reset = is_tty ? "\033[0m" : "";

	std::setlocale(LC_ALL, "");

	// Print the source file and line/column number
	std::cerr << color_purple << source_file << color_reset << ":"
		<< std::to_string(line) << ":"
		<< std::to_string(column) << ": "
		<< std::endl;
	
	// Print the include chain that led to the problematic file
	while (!include_chain.empty()) {
		std::cerr << "In file included from " << color_purple << include_chain.top() << color_reset << std::endl;
		include_chain.pop();
	}

	// Print the warning / error message
	if (is_warning) {
		std::cerr << color_orange << "warning: " << color_reset << msg << std::endl;
	} else {
		std::cerr << color_red << "error: " << color_reset << msg << std::endl;
	}
	
	// Open the source file for reading
	std::ifstream file(source_file);
	if (!file.is_open()) {
		return;
	}

	// Calculate the prefixes:
	// 1. ' (line number) | '
	// 2. '               | '
	std::string line1_prefix = std::to_string(line) + " | ";
	std::string line2_prefix;
	for (size_t i = 0; i < std::to_string(line).size(); i++) {
		line2_prefix += " ";
	}
	line2_prefix += " | ";

	// Read the line with the error
	std::string line_content;
	for (uint32_t i = 0; i <= line; i++) {
		std::getline(file, line_content);
	}
	file.close();

	// Print the line with the error
	std::string line_before_error = utf8_substr(line_content, 0, column);
	std::string error_portion = utf8_substr(line_content, column, text_length);
	uint32_t line_after_error_length = utf8_length(line_content) - (utf8_length(line_before_error) + utf8_length(error_portion));
	std::string line_after_error = utf8_substr(line_content, column + text_length, line_after_error_length);
	
	std::cerr << line1_prefix
		<< line_before_error
		<< (is_warning ? color_orange : color_red) << error_portion << color_reset
		<< line_after_error << std::endl;

	// Print the caret line
	line2_prefix += equal_width_padding(line_before_error);

	std::cerr << line2_prefix
		<< (is_warning ? color_orange : color_red) << "^"
		<< equal_width_padding(utf8_substr(error_portion, 0, utf8_length(error_portion) - 1), '~')
		<< color_reset << std::endl;
}

void print_syntax_error_from_parser(
	std::string source_file,
	uint32_t line,
	uint32_t start_column,
	uint32_t end_column,
	const std::string& msg,
	std::stack<std::string> include_chain
) {
	print_syntax_error_or_warning(
		source_file,
		line,
		start_column,
		end_column - start_column,
		msg,
		include_chain,
		nullptr,
		false,
		false);
}

std::string utf8_substr(const std::string& str, uint32_t start, uint32_t length) {
	std::string::const_iterator it = str.begin();
	
	// Fast-forward the iterator to the start position
	for (uint32_t i = 0; i < start && it != str.end(); ++i) {
		try {
			utf8::next(it, str.end());
		} catch (const utf8::invalid_utf8&) {
			++it;
		}
	}

	std::string result;

	while (it != str.end() && length > 0) {
		uint32_t cp;
		try {
			cp = utf8::next(it, str.end());
		} catch (const utf8::invalid_utf8&) {
			++it;
			continue;
		}
		utf8::append(cp, std::back_inserter(result));
		length--;
	}

	return result;
}

uint32_t utf8_length(const std::string& str) {
	uint32_t length = 0;
	std::string::const_iterator it = str.begin();
	while (it != str.end()) {
		try {
			utf8::next(it, str.end());
			length++;
		} catch (const utf8::invalid_utf8&) {
			++it; // Skip invalid characters
		}
	}
	return length;
}

std::string equal_width_padding(const std::string& str, char padding_char) {
	std::string result;

	std::string::const_iterator it = str.begin();
	while (it != str.end()) {
		uint32_t cp;
		try {
			cp = utf8::next(it, str.end());
		} catch (const utf8::invalid_utf8&) {
			++it;
			continue;
		}
		
		if (cp == '\t') {
			result += "\t"; // TODO(@rail5): Handling tabs this way is much more robust than trying to calculate the width of a tab character
				// But, it also interrupts padding_char unless padding_char is whitespace.
			continue;
		}

		int width = wcwidth(static_cast<wchar_t>(cp));
		if (width < 0) {
			width = 1; // Treat invalid characters as width 1
		}

		for (int i = 0; i < width; i++) {
			result += padding_char;
		}
	}

	return result;
}
