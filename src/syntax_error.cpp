/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_SYNTAX_ERROR_CPP_
#define SRC_SYNTAX_ERROR_CPP_

#include <iostream>
#include <string>
#include <fstream>
#include <stack>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

void print_syntax_error_or_warning(std::string source_file, int line, int column, const std::string& text, const std::string& msg, std::stack<std::string> include_chain, bool is_warning = false) {
	bool is_tty = isatty(fileno(stderr));
	std::string color_red = is_tty ? "\033[0;31m" : "";
	std::string color_purple = is_tty ? "\033[1;35m" : "";
	std::string color_orange = is_tty ? "\033[0;33m" : "";
	std::string color_reset = is_tty ? "\033[0m" : "";
	if (!isatty(fileno(stderr))) {
		color_red = "";
		color_reset = "";
	}

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
		int terminal_width = w.ws_col - firstline_padding.length() - 1;

		// Get the line with the error
		std::string line_contents;
		for (int i = 0; i < line; i++) {
			std::getline(file, line_contents);
		}

		if (static_cast<int>(line_contents.length()) >= terminal_width) {
			// Cut out just enough to show where the error is
			int start = column - (terminal_width / 2);
			if (start < 0) {
				start = 0;
			}
			int end = start + terminal_width;
			if (end > static_cast<int>(line_contents.length())) {
				end = line_contents.length();
				start = end - terminal_width;
			}
			if (start < 0) {
				start = 0;
			}
			line_contents = line_contents.substr(start, end - start);
			column = column - start;
		}
		std::cerr << firstline_padding << line_contents << std::endl;

		for (int i = 0; i < column; i++) {
			if (line_contents[i] == '\t') {
				secondline_padding += "\t";
			} else {
				secondline_padding += " ";
			}
		}

		// Print the error indicator
		std::cerr << secondline_padding << color_red << "^" << color_reset << std::endl;

		file.close();
	}
}

#endif // SRC_SYNTAX_ERROR_CPP_
