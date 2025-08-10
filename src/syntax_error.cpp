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

#include "thirdparty/utf8/utf8.h"

void print_syntax_error_or_warning(
	std::string source_file,
	int line, int column,
	const std::string& text,
	const std::string& msg,
	std::stack<std::string> include_chain,
	std::shared_ptr<bpp::bpp_program> program,
	bool is_warning
) {
	bool is_tty = isatty(fileno(stderr));
	std::string color_red = is_tty ? "\033[0;31m" : "";
	std::string color_purple = is_tty ? "\033[1;35m" : "";
	std::string color_orange = is_tty ? "\033[0;33m" : "";
	std::string color_reset = is_tty ? "\033[0m" : "";
	if (!isatty(fileno(stderr))) {
		color_red = "";
		color_reset = "";
	}

	std::setlocale(LC_ALL, "");

	// Sometimes ANTLR sets position data for a token as line=0, col=-1
	// I have no idea why it does this. I would guess that it's a bug in ANTLR.
	// In those cases, we can still get the token's text
	// Although, unfortunately, ANTLR doesn't give us enough information to give full context
	// Nevertheless, we should display what we have: "Position Unknown" and the text of the token which caused the error.
	bool position_unknown = (line == 0 && column == -1);
	std::string position_info = position_unknown ? "Position Unknown" : (std::to_string(line) + ":" + std::to_string(column));
	std::string extra_info = position_unknown ? (": `" + text + "`") : "";

	std::cerr << color_purple << source_file << color_reset << ":" << position_info << ": " << std::endl;
	while (!include_chain.empty()) {
		std::cerr << "In file included from " << color_purple << include_chain.top() << color_reset << std::endl;
		include_chain.pop();
	}
	if (is_warning) {
		std::cerr << color_orange << "warning: " << color_reset << msg << extra_info << std::endl;
	} else {
		std::cerr << color_red << "error: " << color_reset << msg << extra_info << std::endl;
	}

	// If we do have position data, however, we can give a broader context:
	// Show the contents of the line which has the error,
	// And point an arrow at the token which caused the error
	if (!position_unknown) {
		// Open the source file for reading
		std::ifstream file(source_file);
		if (!file.is_open()) {
			return;
		}

		int number_of_digits = 0;
		int line_copy = line;
		while (line_copy) {
			line_copy /= 10;
			number_of_digits++;
		}

		std::string firstline_padding = std::to_string(line) + " | ";
		std::string secondline_padding = "";
		for (int i = 0; i < number_of_digits; i++) {
			secondline_padding += " ";
		}
		secondline_padding += " | ";

		// Get the current terminal width
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		uint64_t terminal_width = w.ws_col - firstline_padding.length() - 1;

		// Get the line with the error
		std::string line_contents;
		for (int i = 0; i < line; i++) {
			std::getline(file, line_contents);
		}

		if (static_cast<uint64_t>(line_contents.length()) >= terminal_width) {
			// Cut out just enough to show where the error is
			int start = column - static_cast<int>((terminal_width / 2));
			if (start < 0) {
				start = 0;
			}
			int end = start + static_cast<int>(terminal_width);
			if (end > static_cast<int>(line_contents.length())) {
				end = static_cast<int>(line_contents.length());
				start = end - static_cast<int>(terminal_width);
			}
			if (start < 0) {
				start = 0;
			}
			line_contents = line_contents.substr(static_cast<size_t>(start), static_cast<size_t>(end - start));
			column = column - start;
		}

		std::cerr << firstline_padding;

		// Print the line contents up to the start of the error
		std::cerr << line_contents.substr(0, static_cast<size_t>(column));

		// Print the error text
		std::cerr << color_red << line_contents.substr(static_cast<size_t>(column), text.length()) << color_reset;

		// Print the rest of the line contents
		std::cerr << line_contents.substr(static_cast<std::string::size_type>(static_cast<std::string::size_type>(column) + text.length())) << std::endl;

		int display_col = 0;
		std::string::iterator it = line_contents.begin();
		while (display_col < column && it != line_contents.end()) {
			uint32_t cp;
			try {
				cp = utf8::next(it, line_contents.end());
			} catch (const utf8::invalid_utf8& e) {
				// If we encounter an invalid UTF-8 sequence, we can just skip it
				// This is a bit of a hack, but it allows us to continue displaying the rest of the line
				cp = 0xFFFD; // Replacement character
			}

			if (cp == '\t') {
				secondline_padding += "\t";
				display_col++;
				continue;
			}
			
			int width = wcwidth(static_cast<wchar_t>(cp));
			if (width < 0) {
				width = 1;
			}

			for (int i = 0; i < width; i++) {
				secondline_padding += " ";
			}

			display_col++;
		}

		// Print the error indicator
		std::cerr << secondline_padding << color_red << "^";

		// Add an underline for the rest length of the error text
		for (size_t i = 1; i < text.length(); i++) {
			std::cerr << "~";
		}
		
		std::cerr << color_reset << std::endl;

		file.close();
	}

	// Add to the program's diagnostics
	if (!position_unknown && program != nullptr) {
		program->add_diagnostic(
			source_file,
			is_warning ? bpp::diagnostic_type::DIAGNOSTIC_WARNING : bpp::diagnostic_type::DIAGNOSTIC_ERROR,
			msg,
			static_cast<uint32_t>(line) - 1, // 0-indexed. Antlr4 uses 1-indexed line numbers
			static_cast<uint32_t>(column),
			static_cast<uint32_t>(line) - 1,
			static_cast<uint32_t>(column + text.length())
		);
	}
}
