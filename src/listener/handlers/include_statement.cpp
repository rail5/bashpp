/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	// Get the current code entity
	std::shared_ptr<bpp::bpp_program> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->AT(), "Include statements can only be used at the top level of a program");
	}

	std::string filename = ctx->string()->getText();
	filename = filename.substr(1, filename.length() - 2);

	// Is this a relative path?
	// If so, we should search for the file in the same directory as the current source file
	// NOT NECESSARILY the same as the current working directory
	if (filename[0] != '/') {
		// Get the directory of the current source file
		// If the program is being read from stdin, we should use the current working directory
		std::string current_directory;
		if (source_file == "<stdin>") {
			char current_working_directory[PATH_MAX];
			if (getcwd(current_working_directory, PATH_MAX) == nullptr) {
				throw internal_error("Could not get current working directory");
			}
			current_directory = current_working_directory;
		} else {
			current_directory = source_file.substr(0, source_file.find_last_of('/'));
		}

		// Append the filename to the directory
		filename = current_directory + "/" + filename;
	}

	// Get the full path of the file
	char full_path[PATH_MAX];
	if (realpath(filename.c_str(), full_path) == nullptr) {
		throw_syntax_error(ctx->AT(), "File not found: " + filename);
	}

	if (ctx->KEYWORD_INCLUDE_ONCE() != nullptr && included_files.find(full_path) != included_files.end()) {
		// Tell the current parser to skip parsing the inner 'string' rule
		ctx->string()->parent = nullptr;
		ctx->children.clear();
		return;
	}

	included_files.insert(full_path);

	// Create a new listener
	BashppListener listener;
	listener.set_source_file(full_path);
	listener.set_included(true);
	listener.set_included_from(this);
	listener.set_run_on_exit(false);
	listener.set_supershell_counter(supershell_counter);
	listener.set_output_stream(output_stream);
	listener.set_output_file("");

	// Create a new ANTLR input stream
	std::ifstream file_stream(full_path);
	antlr4::ANTLRInputStream input(file_stream);
	BashppLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);

	tokens.fill();

	BashppParser parser(&tokens);

	// Remove default error listeners
	parser.removeErrorListeners();
	// Add diagnostic error listener
	std::unique_ptr<antlr4::DiagnosticErrorListener> error_listener = std::make_unique<antlr4::DiagnosticErrorListener>();
	parser.addErrorListener(error_listener.get());

	// Enable the parser to use diagnostic messages
	parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

	antlr4::tree::ParseTree* tree = nullptr;
	try {
		tree = parser.program();
		// Walk the tree
		antlr4::tree::ParseTreeWalker walker;
		walker.walk(&listener, tree);
	} catch (const antlr4::EmptyStackException& e) {
		std::cerr << "Empty stack exception from included file '" << full_path << "'" << std::endl;
		throw antlr4::EmptyStackException(e);
	} catch (const antlr4::RecognitionException& e) {
		std::cerr << "Recognition exception from included file '" << full_path << "'" << std::endl;
		throw antlr4::RecognitionException(e);
	} catch (const internal_error& e) {
		std::cerr << "Internal error from included file '" << full_path << "'" << std::endl;
		throw internal_error(e);
	} catch (const std::exception& e) {
		std::cerr << "Standard exception from included file '" << full_path << "'" << std::endl;
		throw std::exception(e);
	} catch (...) {
		std::cerr << "Unknown exception occurred (from included file '" << full_path << "')" << std::endl;
		throw;
	}

	// The code, objects, classes, etc should all have been added to the current program by the included program's new listener

	// Tell the current parser to skip parsing the inner 'string' rule
	ctx->string()->parent = nullptr;
	ctx->children.clear();
}

void BashppListener::exitInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_
