/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Diagnostic.h"

#include <IR/SourceIndex.h>

#include <utf8cpp/utf8.h>

#include <iostream>
#include <fstream>
#include <string_view>
#include <limits>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

namespace {

inline std::string utf8_substr(const std::string& str, std::uint32_t start, std::uint32_t length) {
	std::string::const_iterator it = str.begin();

	// Fast-forward the iterator to the start position
	for (std::uint32_t i = 0; i < start && it != str.end(); ++i) {
		try {
			utf8::next(it, str.end());
		} catch (const utf8::invalid_utf8&) {
			++it;
		}
	}

	std::string result;

	while (it != str.end() && length > 0) {
		std::uint32_t cp = 0;
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

inline std::uint32_t utf8_length(const std::string& str) {
	std::uint32_t length = 0;
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

inline std::string equal_width_padding(const std::string& str, char padding_char) {
	std::string result;

	std::string::const_iterator it = str.begin();
	while (it != str.end()) {
		std::uint32_t cp = 0;
		try {
			cp = utf8::next(it, str.end());
		} catch (const utf8::invalid_utf8&) {
			++it;
			continue;
		}

		if (cp == '\t') {
			result += '\t'; // TODO(@rail5): Handling tabs this way is much more robust than trying to calculate the width of a tab character
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

} // anonymous namespace

namespace bpp::ErrorHandling {

void Diagnostic::print_to_stderr() const {
	bool is_tty = isatty(fileno(stderr));
	const std::string_view color_red = is_tty ? "\033[0;31m" : "";
	const std::string_view color_purple = is_tty ? "\033[1;35m" : "";
	const std::string_view color_orange = is_tty ? "\033[0;33m" : "";
	const std::string_view color_green = is_tty ? "\033[0;32m" : "";
	const std::string_view color_reset = is_tty ? "\033[0m" : "";

	const std::string_view& display_color = (type == DiagnosticType::DIAGNOSTIC_WARNING) ? color_orange : color_red;

	// First line: `/path/to/source/file.bpp:line:column:`
	std::cerr << color_purple << include_chain.back().string() << color_reset << ":"
		<< std::to_string(line + 1) << ":"
		<< std::to_string(column + 1) << ":"
		<< std::endl;

	// Next line: include chain, printed in reverse order, excluding the last element (the file that produced the diagnostic)
	for (auto it = ++include_chain.rbegin(); it != include_chain.rend(); ++it) {
		std::cerr << "In file included from " << color_purple << it->string() << color_reset << std::endl;
	}

	// Next line:
	// `[warning|error|info|hint]: <message> [-W<warning_cli_string>]`
	switch (type) {
		case DiagnosticType::DIAGNOSTIC_WARNING:
			std::cerr << color_orange << "warning: " << color_reset;
			break;
		case DiagnosticType::DIAGNOSTIC_ERROR:
			std::cerr << color_red << "error: " << color_reset;
			break;
		case DiagnosticType::DIAGNOSTIC_INFO:
			std::cerr << color_purple << "info: " << color_reset;
			break;
		case DiagnosticType::DIAGNOSTIC_HINT:
			std::cerr << color_green << "hint: " << color_reset;
			break;
	}

	std::cerr << this->what();

	if (warning_cli_string) {
		std::cerr << " [" << color_orange << "-W" << *warning_cli_string << color_reset << "]";
	}

	std::cerr << std::endl;

	std::ifstream file(include_chain.back());
	if (!file.is_open()) return; // If we can't open the file for reading, the above information will just have to suffice.

	// Calculate the prefixes for lines from the file:
	// 1. ' (line number) | '
	// 2. '               | '
	std::string line1_prefix = std::to_string(line + 1);
	std::string line2_prefix(line1_prefix.size(), ' ');
	line1_prefix += " | ";
	line2_prefix += " | ";

	// Skip to the line with the error
	for (std::uint32_t i = 0; i < line; i++) file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// Read the line with the error
	std::string line_content;
	std::getline(file, line_content);

	// Print the line with the error, highlighting the error portion
	std::string text_before_error = utf8_substr(line_content, 0, column);
	std::string error_portion = utf8_substr(line_content, column, error_text_length);
	std::uint32_t text_after_error_length = utf8_length(line_content) - (utf8_length(text_before_error) + utf8_length(error_portion));
	std::string text_after_error = utf8_substr(line_content, column + error_text_length, text_after_error_length);

	std::cerr << line1_prefix
		<< text_before_error
		<< display_color << error_portion << color_reset
		<< text_after_error
		<< std::endl;

	// Now print the line with the caret pointing to the error portion
	line2_prefix += equal_width_padding(text_before_error, ' ');

	std::cerr << line2_prefix
		<< display_color << "^"
		<< equal_width_padding(utf8_substr(error_portion, 0, utf8_length(error_portion) - 1), '~')
		<< color_reset
		<< std::endl;
}

} // namespace bpp::ErrorHandling
