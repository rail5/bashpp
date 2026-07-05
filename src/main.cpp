/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
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

#include <iostream>
#include <cstring>
#include <memory>
#include <unistd.h>
#include <cstdlib>

#include <version.h>
#include <updated_year.h>

#include <include/parse_arguments.h>
#include <include/run_bash.h>
#include <AST/Parser.h>
#include <AST/Listener/Listener.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

int main(int argc, char** argv) {
	(void)std::setlocale(LC_ALL, ""); // NOLINT (concurrency-mt-unsafe)

	Arguments args;
	std::shared_ptr<std::ostream> output_stream;
	try {
		args = parse_arguments(argc, argv);

		if (args.exit_early()) return 0;

		output_stream = determine_output_stream(&args);
	} catch (const std::exception& e) {
		std::cerr << program_name << ": Error: " << e.what() << std::endl
			<< "Use -h for help" << std::endl;
		return 1;
	}

	// If the user didn't provide input, let them know, rather than just hang waiting for stdin
	if (args.input_from_stdin() && isatty(STDIN_FILENO)) {
		std::cerr << program_name << " " << bpp_compiler_version << std::endl
			<< help_intro << OptionParser.getHelpString();
		return 1;
	}

	std::unique_ptr<bpp::AST::Listener> listener = std::make_unique<bpp::AST::Listener>();

	bpp::AST::Parser parser;
	if (args.input_from_stdin()) {
		parser.setInputFromFilePtr(stdin, "<stdin>");
	} else {
		parser.setInputFromFilePath(args.input_file().value());
	}
#ifndef NDEBUG
	parser.setDisplayLexerOutput(args.display_tokens());
#endif
	
	auto program = parser.program();
	for (const auto& e : parser.get_errors()) {
		e.print();
		listener->set_has_errors(true);
	}

	if (program == nullptr) {
		std::cerr << program_name << ": Error: Failed to parse program." << std::endl;
		return 1;
	}

#ifndef NDEBUG
	if (args.display_parse_tree()) {
		// '-p' given, exit after displaying the parse tree
		std::cout << *program << std::endl;
		if (!args.display_entity_tree()) return 0; // Exit unless we have to build/display the entity tree
	} else if (args.display_tokens()) {
		// '-t' given (lexer tokens displayed), exit now even if we didn't display the parse tree
		if (!args.display_entity_tree()) return 0;
	}
#endif

	listener->set_source_file(args.input_file().value_or("<stdin>"));
	listener->set_warning_options(args.warning_options());
	listener->set_include_paths(args.include_paths());

	try {
		listener->walk(program.get());

		if (listener->has_errors()) return 1;
	#ifndef NDEBUG
		if (args.display_entity_tree()) {
			std::cout << *listener->get_program() << std::endl;
			return 0;
		}
	#endif
		bpp::CodeGen::CodeGenState codegen_state;
		codegen_state.target_bash_version = args.target_bash_version();
		*output_stream << listener->get_program()->generate_code(&codegen_state);

		int exit_code = 0;

		if (args.run_on_exit()) {
			exit_code = run_bash(args.output_file().value(), args.program_arguments());
		} else if (args.output_to_file()) {
			// Mark the file executable
			try {
				std::filesystem::permissions(
					args.output_file().value(),
					std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
					std::filesystem::perm_options::add
				);
			} catch (...) { /* ignore */ }
		}

		return exit_code;
	} catch (const bpp::ErrorHandling::InternalError& e) {
		std::cerr << "Internal error: " << e.what() << std::endl;
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
		std::cerr << "Exception type: " << typeid(e).name() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
		return 1;
	}
}
