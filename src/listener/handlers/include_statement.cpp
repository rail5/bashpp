/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	// Get the current code entity
	std::shared_ptr<bpp::bpp_program> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());

	antlr4::tree::TerminalNode* initial_node = nullptr;

	if (ctx->KEYWORD_INCLUDE() != nullptr) {
		initial_node = ctx->KEYWORD_INCLUDE();
	} else if (ctx->KEYWORD_INCLUDE_ONCE() != nullptr) {
		initial_node = ctx->KEYWORD_INCLUDE_ONCE();
	}

	if (current_code_entity == nullptr) {
		throw_syntax_error(initial_node, "Include statements can only be used at the top level of a program");
	}

	std::string filename = ctx->INCLUDE_PATH()->getText();

	if (ctx->LOCAL_INCLUDE_PATH_START() == nullptr) {
		// Search for the file in the include paths
		bool found = false;
		for (const std::string& include_path : *include_paths) {
			std::string full_path = include_path + "/" + filename;
			if (access(full_path.c_str(), F_OK) == 0) {
				filename = full_path;
				found = true;
				break;
			}
		}
		if (!found) {
			throw_syntax_error(initial_node, "File not found: " + filename);
		}
	} else {
		// This is a "local" include -- meaning we should start scanning from the same directory as the current source file
		// Unless the path given is an absolute path (starting with '/')
		// NOT NECESSARILY the same as the current working directory
		if (filename[0] != '/') {
			// Get the directory of the current source file
			// If the program is being read from stdin, we should use the current working directory
			std::string current_directory;
			if (source_file == "<stdin>") {
				char current_working_directory[PATH_MAX];
				if (getcwd(current_working_directory, PATH_MAX) == nullptr) {
					throw internal_error("Could not get current working directory", ctx);
				}
				current_directory = current_working_directory;
			} else {
				current_directory = source_file.substr(0, source_file.find_last_of('/'));
			}

			// Append the filename to the directory
			filename = current_directory + "/" + filename;
		}
	}
	// Get the full path of the file
	char full_path[PATH_MAX];
	if (realpath(filename.c_str(), full_path) == nullptr) {
		throw_syntax_error(initial_node, "File not found: " + filename);
	}

	if (ctx->KEYWORD_INCLUDE_ONCE() != nullptr && included_files.find(full_path) != included_files.end()) {
		ctx->children.clear();
		return;
	}

	included_files.insert(full_path);

	// Create a new listener
	BashppListener listener;
	listener.set_source_file(full_path);
	listener.set_include_paths(include_paths);
	listener.set_included(true);
	listener.set_included_from(this);
	listener.set_run_on_exit(false);
	listener.set_suppress_warnings(suppress_warnings);

	if (!dynamic_linking) {
		// If we're linking statically, copy the compiled code from the included file to the current program
		listener.set_output_stream(output_stream);
	} else {
		// Otherwise, throw its output in the garbage
		std::shared_ptr<std::ofstream> garbage_stream = std::make_shared<std::ofstream>("/dev/null");
		listener.set_output_stream(garbage_stream);
	}
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

	// The objects, classes, etc should all have been added to the current program by the included program's new listener
	ctx->children.clear();

	// Recover our original output stream
	program->set_output_stream(output_stream);

	// If we're linking statically, the code was also added
	// If we're linking dynamically, we need to add a little source directive here
	if (dynamic_linking) {
		// If the 'full path' has an extension, we should remove it
		std::string full_path_str = full_path;
		// Get the file basename
		std::string basename = full_path_str.substr(full_path_str.find_last_of('/') + 1);
		std::string directory = full_path_str.substr(0, full_path_str.find_last_of('/'));
		// Get the file extension
		std::string extension = basename.substr(basename.find_last_of('.') + 1);

		// If the extension exists, remove it
		if (extension.length() > 0) {
			basename = basename.substr(0, basename.find_last_of('.'));
		}

		// Append a '.sh' extension
		basename += ".sh";
		
		// Add a source directive
		current_code_entity->add_code("source \"" + directory + "/" + basename + "\"\n");
	}
}

void BashppListener::exitInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_
