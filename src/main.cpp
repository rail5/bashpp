/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

/*! \mainpage
 * \section intro_sec Introduction
 *
 * src/main.cpp is not necessarily the best introduction to the compiler's code.
 * 
 * Here is an article which gives an overview of the compiler:
 * 
 * [How Does the Bash++ Compiler Work?](https://log.bpp.sh/2025/05/16/how-does-the-bashpp-compiler-work.html)
 *
 * As for the rest, the Doxygen documentation will provide details on the compiler's different classes and functions.
*/

#include "exit_code.h"
volatile int bpp_exit_code = 0;

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <filesystem>
#include <cstring>
#include <memory>
#include <antlr4-runtime.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>

#include "version.h"
#include "updated_year.h"

#include "syntax_error.h"

#include "antlr/BashppLexer.h"
#include "antlr/BashppParser.h"

#include "listener/BashppListener.h"

#include "internal_error.h"

int main(int argc, char* argv[]) {
	constexpr const char* program_name = "Bash++";
	constexpr const char* copyright = "Copyright (C) 2024-"
		bpp_compiler_updated_year
		" Andrew S. Rightenburg\n\n"
		"This program is free software; you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation; either version 3 of the License, or\n"
		"(at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program. If not, see http://www.gnu.org/licenses/.\n";
	constexpr const char* help_string = "Usage: bpp [options] [file] ...\n"
		"If no file is specified, read from stdin\n"
		"All arguments after the file are passed to the compiled program\n"
		"Options:\n"
		"  -o, --output <file>   Specify output file\n"
		"                         If not specified, program will run on exit\n"
		"                         If specified as '-', program will be written to stdout\n"
		"  -s, --no-warnings     Suppress warnings\n"
		"  -I, --include <path>  Add directory to include path\n"
		"  -p, --parse-tree      Display parse tree (do not compile program)\n"
		"  -t, --tokens          Display tokens (do not compile program)\n"
		"  -v, --version         Print version information\n"
		"  -h, --help            Print this help message\n";
	int c;
	opterr = 0;
	int option_index = 0;

	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"include", required_argument, 0, 'I'},
		{"output", required_argument, 0, 'o'},
		{"parse-tree", no_argument, 0, 'p'},
		{"no-warnings", no_argument, 0, 's'},
		{"tokens", no_argument, 0, 't'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0} // Sentinel
	};

	bool received_filename = false;
	bool received_output_filename = false;
	bool run_on_exit = true;
	bool suppress_warnings = false;
	std::string file_to_read = "";
	std::string output_file = "";

	bool display_parse_tree = false;
	bool display_tokens = false;

	std::shared_ptr<std::ostream> output_stream(&std::cout, [](std::ostream*){});
	std::shared_ptr<std::ostringstream> code_buffer = std::make_shared<std::ostringstream>();

	std::shared_ptr<std::vector<std::string>> include_paths = std::make_shared<std::vector<std::string>>();
	include_paths->push_back("/usr/lib/bpp/stdlib/");

	std::vector<char*> program_arguments = {};
	std::vector<char*> compiler_arguments = {};
	program_arguments.reserve(static_cast<std::vector<char*>::size_type>(argc));
	compiler_arguments.reserve(static_cast<std::vector<char*>::size_type>(argc));
	compiler_arguments.push_back(argv[0]);

	// Find the first non-option argument, interpret it as the source file to compile,
	// And store any subsequent arguments as arguments to the program
	for (int i = 1; i < argc; i++) {
		if (!received_filename && argv[i][0] != '-' && strcmp(argv[i-1], "-o") != 0) {
			file_to_read = argv[i];
			received_filename = true;
		} else if (received_filename) {
			program_arguments.push_back(argv[i]);
		} else {
			// Add the argument to the compiler_arguments array
			compiler_arguments.push_back(argv[i]);
		}
	}

	while (
		(c = getopt_long(
			static_cast<int>(compiler_arguments.size()),
			compiler_arguments.data(),
			"DhI:o:pstv",
			long_options,
			&option_index)
		) != -1
	) {
		switch(c) {
			case 'h':
				std::cout << program_name << " " << bpp_compiler_version << std::endl << help_string;
				return 0;
				break;
			case 'I':
				// Verify the given include path is a directory
				if (!std::filesystem::exists(optarg) || !std::filesystem::is_directory(optarg)) {
					std::cerr << program_name << ": Error: Include path '" << optarg << "' does not exist or is not a directory" << std::endl;
					return 1;
				}
				include_paths->push_back(std::string(optarg));
				break;
			case 'o':
				if (received_output_filename) {
					std::cerr << program_name << ": Error: Multiple output files specified" << std::endl;
					return 1;
				}
				run_on_exit = false;
				received_output_filename = true;

				output_file = std::string(optarg);

				if (output_file == "-") {
					break;
				}

				// Check if we have permission to write to the specified output file
				// If the file already exists, verify write access; if it doesn't, verify write access on its parent directory
				try {
					if (std::filesystem::exists(output_file)) {
						if (!std::filesystem::is_regular_file(output_file)) {
							std::cerr << program_name << ": Error: Output file '" << output_file << "' is not a regular file" << std::endl;
							return 1;
						}
						if (access(output_file.c_str(), W_OK) != 0) {
							std::cerr << program_name << ": Error: No write permission for output file '" << output_file << "'" << std::endl;
							return 1;
						}
					} else {
						std::filesystem::path parent_path = std::filesystem::path(output_file).parent_path();
						if (!std::filesystem::exists(parent_path) || !std::filesystem::is_directory(parent_path)) {
							std::cerr << program_name << ": Error: Parent directory of output file '" << output_file << "' does not exist or is not a directory" << std::endl;
							return 1;
						}
						if (access(parent_path.c_str(), W_OK) != 0) {
							std::cerr << program_name << ": Error: No write permission for parent directory of output file '" << output_file << "'" << std::endl;
							return 1;
						}
					}
				} catch (...) {
					std::cerr << program_name << ": Error: Could not verify write permission for output file '" << output_file << "'" << std::endl;
					return 1;
				}
				output_stream = std::dynamic_pointer_cast<std::ostream>(std::make_shared<std::ofstream>(output_file));
				if (output_stream == nullptr) {
					std::cerr << program_name << ": Error: Could not open file " << output_file << " for output" << std::endl;
					return 1;
				}
				break;
			case 'p':
				display_parse_tree = true;
				break;
			case 's':
				suppress_warnings = true;
				break;
			case 't':
				display_tokens = true;
				break;
			case 'v':
				std::cout << program_name << " " << bpp_compiler_version << std::endl << copyright;
				return 0;
				break;
			case '?':
				if (optopt == 'o') {
					std::cerr << program_name << ": Option -" << static_cast<char>(optopt) << " requires an argument" << std::endl << "Use -h for help" << std::endl;
					return 1;
				}
				break;
		}
	}

	std::ifstream file_stream;
	std::istream* stream = &std::cin;

	if (received_filename) {
		// Verify that the file exists, is readable, and is a regular file
		if (!std::filesystem::exists(file_to_read)) {
			std::cerr << program_name << ": Error: Source file '" << file_to_read << "' does not exist" << std::endl;
			return 1;
		}
		if (!std::filesystem::is_regular_file(file_to_read)) {
			std::cerr << program_name << ": Error: Source file '" << file_to_read << "' is not a regular file" << std::endl;
			return 1;
		}
		file_stream.open(file_to_read);
		if (!file_stream.is_open()) {
			std::cerr << program_name << ": Error: Could not open source file '" << file_to_read << "'" << std::endl;
			return 1;
		}
		stream = &file_stream;
	}

	if (!stream) {
		std::cerr << program_name << ": Error: Could not open source file" << std::endl;
		return 1;
	}

	/* If the user didn't provide input, let them know, rather than just hang waiting for cin */
	if (stream->rdbuf() == std::cin.rdbuf() && isatty(fileno(stdin))) {
		std::cerr << program_name << " " << bpp_compiler_version << std::endl << help_string;
		return 1;
	}

	antlr4::ANTLRInputStream input(*stream);
	BashppLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);

	tokens.fill();

	if (display_tokens) {
		for (auto token : tokens.getTokens()) {
			std::cout << "Token: " << token->getText() 
					  << ", Type: " << lexer.getVocabulary().getSymbolicName(token->getType()) 
					  << std::endl;
		}
		if (!display_parse_tree) {
			return bpp_exit_code;
		}
	}

	BashppParser parser(&tokens);

	// Remove default error listeners
	parser.removeErrorListeners();
	// Add diagnostic error listener
	std::unique_ptr<antlr4::DiagnosticErrorListener> error_listener = std::make_unique<antlr4::DiagnosticErrorListener>();
	parser.addErrorListener(error_listener.get());

	// Enable the parser to use diagnostic messages
	parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

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
		std::shared_ptr<std::ofstream> stream = std::make_shared<std::ofstream>(temp_file_vec.data());
		if (!stream->is_open()) {
			std::cerr << program_name << ": Error: Could not open temporary file for output" << std::endl;
			return 1;
		}
		output_stream = std::dynamic_pointer_cast<std::ostream>(stream);
		output_file = std::string(temp_file_vec.data());
	}

	antlr4::tree::ParseTree* tree = nullptr;
	try {
		tree = parser.program();

		if (display_parse_tree) {
			std::cout << tree->toStringTree(&parser, true) << std::endl;
			return bpp_exit_code;
		}

		// Walk the tree
		antlr4::tree::ParseTreeWalker walker;
		std::unique_ptr<BashppListener> listener = std::make_unique<BashppListener>();
		std::string full_path;
		char resolved_path[PATH_MAX];
		if (realpath(file_to_read.c_str(), resolved_path) == nullptr) {
			if (!received_filename) {
				full_path = "<stdin>";
			} else {
				std::cerr << "Error: Could not get full path of source file '" << file_to_read << "'" << std::endl;
				return 1;
			}
		} else {
			full_path = std::string(resolved_path);
		}
		listener->set_source_file(full_path);
		listener->set_include_paths(include_paths);
		listener->set_code_buffer(code_buffer);
		listener->set_output_stream(output_stream);
		listener->set_output_file(output_file);
		listener->set_run_on_exit(run_on_exit);
		listener->set_suppress_warnings(suppress_warnings);
		listener->set_arguments(program_arguments);
		walker.walk(listener.get(), tree);

	} catch (const antlr4::EmptyStackException& e) {
		std::cerr << "EmptyStackException: " << e.what() << std::endl;
	} catch (const antlr4::RecognitionException& e) {
		std::cerr << "Recognition exception: " << e.what() << std::endl;
	} catch (const internal_error& e) {
		std::cerr << "Internal error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
	}

	return bpp_exit_code;
}
