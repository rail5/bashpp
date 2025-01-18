/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <antlr4-runtime.h>
#include <getopt.h>

#include "syntax_error.cpp"

#include "out/BashppLexer.h"
#include "out/BashppParser.h"

#include "listener/BashppListener.h"

#include "listener/handlers/class_body_statement.cpp"
#include "listener/handlers/class_definition.cpp"
#include "listener/handlers/comment.cpp"
#include "listener/handlers/constructor_definition.cpp"
#include "listener/handlers/delete_statement.cpp"
#include "listener/handlers/deprecated_subshell.cpp"
#include "listener/handlers/destructor_definition.cpp"
#include "listener/handlers/general_statement.cpp"
#include "listener/handlers/include_statement.cpp"
#include "listener/handlers/member_declaration.cpp"
#include "listener/handlers/method_definition.cpp"
#include "listener/handlers/new_statement.cpp"
#include "listener/handlers/nullptr_ref.cpp"
#include "listener/handlers/object_address.cpp"
#include "listener/handlers/object_assignment.cpp"
#include "listener/handlers/object_instantiation.cpp"
#include "listener/handlers/object_reference_as_lvalue.cpp"
#include "listener/handlers/object_reference.cpp"
#include "listener/handlers/other_statement.cpp"
#include "listener/handlers/parameter.cpp"
#include "listener/handlers/pointer_declaration.cpp"
#include "listener/handlers/pointer_dereference.cpp"
#include "listener/handlers/program.cpp"
#include "listener/handlers/raw_rvalue.cpp"
#include "listener/handlers/self_reference_as_lvalue.cpp"
#include "listener/handlers/self_reference.cpp"
#include "listener/handlers/singlequote_string.cpp"
#include "listener/handlers/statement.cpp"
#include "listener/handlers/string.cpp"
#include "listener/handlers/subshell.cpp"
#include "listener/handlers/supershell.cpp"
#include "listener/handlers/value_assignment.cpp"

#include "internal_error.cpp"

using namespace antlr4;

int main(int argc, char* argv[]) {
	const char* program_name = "Bash++";
	const char* copyright = "Copyright (C) 2025 Andrew S. Rightenburg\n"
		"GNU GPL v3.0 or later\n";
	std::string help_string = "Usage: " + std::string(argv[0]) + " [options] [file]\n"
		"Options:\n"
		"  -o, --output <file>   Specify output file\n"
		"                         If not specified, program will be run on exit\n"
		"                         If specified as '-', output will be written to stdout\n"
		"  -v, --version         Print version information\n"
		"  -h, --help            Print this help message\n";
	int c;
	opterr = 0;
	int option_index = 0;
	
	static struct option long_options[] = {
		{"output", required_argument, 0, 'o'},
		{"version", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'}
	};

	bool received_filename = false;
	bool received_output_filename = false;
	bool run_on_exit = true;
	std::string file_to_read = "";
	std::string output_file = "";

	std::ostream* output_stream = &std::cout;
	std::shared_ptr<std::ofstream> outfilestream;
	
	while ((c = getopt_long(argc, argv, "o:vh", long_options, &option_index)) != -1) {
		switch(c) {
			case 'o':
				if (received_output_filename) {
					std::cerr << program_name << ": Error: Multiple output files specified" << std::endl;
					return 1;
				}
				run_on_exit = false;
				received_output_filename = true;

				output_file = std::string(optarg);

				if (output_file == "-") {
					output_stream = &std::cout;
					break;
				}

				outfilestream = std::make_shared<std::ofstream>(output_file);
				if (!*outfilestream) {
					std::cerr << program_name << ": Error: Could not open file " << output_file << " for output" << std::endl;
					return 1;
				}
				output_stream = outfilestream.get();
				break;
			case 'v':
				std::cout << program_name << " 0.1" << std::endl << copyright;
				return 0;
				break;
			case 'h':
				std::cout << program_name << " 0.1" << std::endl << copyright << help_string;
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
	
	for (option_index = optind; option_index < argc; option_index++) {
		if (received_filename) {
			std::cerr << program_name << ": Error: Multiple files specified" << std::endl;
			return 1;
		}
		file_to_read = argv[option_index];
		received_filename = true;
	}

	std::ifstream stream(file_to_read);
	if (!stream) {
		std::cerr << "Error: Could not open source file " << file_to_read << std::endl;
		return 1;
	}

	ANTLRInputStream input(stream);
	BashppLexer lexer(&input);
	CommonTokenStream tokens(&lexer);

	tokens.fill();

	BashppParser parser(&tokens);

	// Remove default error listeners
	parser.removeErrorListeners();
	// Add diagnostic error listener
	std::unique_ptr<DiagnosticErrorListener> error_listener = std::make_unique<DiagnosticErrorListener>();
	parser.addErrorListener(error_listener.get());

	// Enable the parser to use diagnostic messages
	parser.setErrorHandler(std::make_shared<BailErrorStrategy>());

	tree::ParseTree* tree = nullptr;
	try {
		tree = parser.program();
		// Walk the tree
		antlr4::tree::ParseTreeWalker walker;
		std::unique_ptr<BashppListener> listener = std::make_unique<BashppListener>();
		char full_path[PATH_MAX];
		if (realpath(file_to_read.c_str(), full_path) == nullptr) {
			std::cerr << "Error: Could not resolve full path for file " << file_to_read << std::endl;
			return 1;
		}
		listener->set_source_file(full_path);
		listener->set_output_stream(output_stream);
		listener->set_output_file(output_file);
		listener->set_run_on_exit(run_on_exit);
		walker.walk(listener.get(), tree);

	} catch (const antlr4::EmptyStackException& e) {
		std::cerr << "EmptyStackException: " << e.what() << std::endl;
	} catch (const RecognitionException& e) {
		std::cerr << "Recognition exception: " << e.what() << std::endl;
	} catch (const internal_error& e) {
		std::cerr << "Internal error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
	}

	return 0;
}