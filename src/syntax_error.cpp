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

void print_syntax_error(std::string source_file, int line, int column, std::string msg, std::stack<std::string> include_chain) {
	std::string color_red = "\033[0;31m";
	std::string color_reset = "\033[0m";
	if (!isatty(fileno(stderr))) {
		color_red = "";
		color_reset = "";
	}

	std::cerr << source_file << ":" << line << ":" << column << ": " << std::endl;
	while (!include_chain.empty()) {
		std::cerr << "In file included from " << include_chain.top() << std::endl;
		include_chain.pop();
	}
	std::cerr << color_red << "error: " << color_reset << msg << std::endl;

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

	if (line_contents.length() >= terminal_width) {
		// Cut out just enough to show where the error is
		int start = column - (terminal_width / 2);
		if (start < 0) {
			start = 0;
		}
		int end = start + terminal_width;
		if (end > line_contents.length()) {
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

#endif // SRC_SYNTAX_ERROR_CPP_