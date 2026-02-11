/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
*/

/*! \mainpage
 * \section intro_sec Introduction
 *
 * src/main.cpp is not necessarily the best introduction to the compiler's code.
 * 
 * Here is an article which gives an overview of the compiler:
 * 
 * [How Does the Bash++ Compiler Work?](https://log.bpp.sh/2026/01/25/how-does-the-bashpp-compiler-work.html)
 *
 * As for the rest, the Doxygen documentation will provide details on the compiler's different classes and functions.
*/

#include <include/exit_code.h>
volatile int bpp_exit_code = 0;

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <memory>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>

#include <version.h>
#include <updated_year.h>

#include <include/parse_arguments.h>
#include <AST/BashppParser.h>
#include <listener/BashppListener.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

int main(int argc, char* argv[]) {
	std::shared_ptr<std::ostream> output_stream(&std::cout, [](std::ostream*){});
	std::shared_ptr<std::ostringstream> code_buffer = std::make_shared<std::ostringstream>();

	Arguments args;
	try {
		args = parse_arguments(argc, argv);
	} catch (const std::runtime_error& e) {
		std::cerr << program_name << ": Error: " << e.what() << std::endl
			<< "Use -h for help" << std::endl;
		return 1;
	}

	if (args.exit_early) {
		return 0;
	}

	bool run_on_exit = false;

	if (args.output_file.has_value() && args.output_file.value() != "-") {
		output_stream = std::make_shared<std::ofstream>(args.output_file.value());
		if (!std::dynamic_pointer_cast<std::ofstream>(output_stream)->is_open()) {
			std::cerr << program_name << ": Error: Could not open output file '" << args.output_file.value() << "'" << std::endl;
			return 1;
		}
	} else if (!args.output_file.has_value()) {
		run_on_exit = true; // If no output file was given, we run the program on exit
	}

	FILE* input_file = stdin;
	std::string input_file_path = "<stdin>";

	if (args.input_file.has_value()) {
		// Verify that the file exists, is readable, and is a regular file
		if (!std::filesystem::exists(args.input_file.value())) {
			std::cerr << program_name << ": Error: Source file '" << args.input_file.value() << "' does not exist" << std::endl;
			return 1;
		}
		if (!std::filesystem::is_regular_file(args.input_file.value())) {
			std::cerr << program_name << ": Error: Source file '" << args.input_file.value() << "' is not a regular file" << std::endl;
			return 1;
		}
		input_file = fopen(args.input_file.value().c_str(), "r");
		input_file_path = args.input_file.value();
		if (input_file == nullptr) {
			std::cerr << program_name << ": Error: Could not open source file '" << args.input_file.value() << "'" << std::endl;
			return 1;
		}
	}

	if (input_file == nullptr) {
		std::cerr << program_name << ": Error: Could not open source file" << std::endl;
		return 1;
	}

	/* If the user didn't provide input, let them know, rather than just hang waiting for cin */
	if (input_file == stdin && isatty(fileno(stdin))) {
		std::cerr << program_name << " " << bpp_compiler_version << std::endl
			<< help_intro << OptionParser.getHelpString();
		if (input_file != nullptr && input_file != stdin) {
			fclose(input_file);
		}
		return 1;
	}

	std::string full_path;
	char resolved_path[PATH_MAX];
	if (realpath(args.input_file.value_or("").c_str(), resolved_path) == nullptr) {
		if (!args.input_file.has_value()) {
			full_path = "<stdin>";
		} else {
			std::cerr << "Error: Could not get full path of source file '" << args.input_file.value() << "'" << std::endl;
			return 1;
		}
	} else {
		full_path = std::string(resolved_path);
	}

	AST::BashppParser parser;
	parser.setInputFromFilePtr(input_file, input_file_path);
	parser.setDisplayLexerOutput(args.display_tokens);
	
	auto program = parser.program();
	const auto& parser_errors = parser.get_errors();
	if (program == nullptr) {
		bpp::ErrorHandling::print_parser_errors(parser_errors, full_path, {}, nullptr, false);
		std::cerr << program_name << ": Error: Failed to parse program." << std::endl;
		return 1;
	}

	if (args.display_parse_tree) {
		// '-p' given, exit after displaying the parse tree
		std::cout << *program << std::endl;
		return 0;
	} else if (args.display_tokens) {
		// '-t' given (lexer tokens displayed), exit now even if we didn't display the parse tree
		return 0;
	}

	if (run_on_exit) {
		// Create a temporary file to store the program
		std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "bashpp_temp_XXXXXX";
		std::string temp_file = temp_path.string();
		// mkstemp requires a mutable char array
		std::vector<char> temp_file_vec(temp_file.begin(), temp_file.end());
		temp_file_vec.push_back('\0'); // Null-terminate the string for mkstemp
		int fd = mkstemp(temp_file_vec.data());
		if (fd == -1) {
			throw std::runtime_error("Failed to create temporary file");
		}
		close(fd);
		std::shared_ptr<std::ofstream> ostream = std::make_shared<std::ofstream>(temp_file_vec.data());
		if (!ostream->is_open()) {
			std::cerr << program_name << ": Error: Could not open temporary file for output" << std::endl;
			return 1;
		}
		output_stream = std::dynamic_pointer_cast<std::ostream>(ostream);
		args.output_file = std::string(temp_file_vec.data());
	}

	try {
		// Walk the tree
		std::unique_ptr<BashppListener> listener = std::make_unique<BashppListener>();
		listener->set_source_file(full_path);
		listener->set_include_paths(args.include_paths);
		listener->set_code_buffer(code_buffer);
		listener->set_output_stream(output_stream);
		listener->set_output_file(args.output_file.value_or(""));
		listener->set_run_on_exit(run_on_exit);
		listener->set_suppress_warnings(args.suppress_warnings);
		listener->set_target_bash_version(args.target_bash_version);
		listener->set_arguments(args.program_arguments);
		listener->set_parser_errors(parser_errors);
		listener->walk(program);

	} catch (const bpp::ErrorHandling::InternalError& e) {
		std::cerr << "Internal error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
		// Output the type of the exception
		std::cerr << "Exception type: " << typeid(e).name() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
	}

	return bpp_exit_code;
}
