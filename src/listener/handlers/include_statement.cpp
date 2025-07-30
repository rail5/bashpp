/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"
#include "../../antlr/BashppLexer.h"
#include "../../antlr/BashppParser.h"

#include <unistd.h>

/**
 * @brief Handles @include and @include_once statements
 * 
 * This function is called when the parser enters an @include or @include_once statement.
 * 
 * The syntax of an include is:
 * 
 * @include [static | dynamic] {PATH} [as "{PATH}"]
 * 
 * For example:
 * 
 * @include_once dynamic <Stack> as "/usr/lib/Stack.sh"
 * 
 * Or:
 * 
 * @include "Stack.bpp"
 */
void BashppListener::enterInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_syntax_errors
	if (ctx->JUNK().size() > 0) {
		throw_syntax_error(ctx->JUNK(0), "Include statement not understood");
	}

	// Get the current code entity
	std::shared_ptr<bpp::bpp_program> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());

	antlr4::tree::TerminalNode* initial_node = nullptr;
	antlr4::tree::TerminalNode* sourcePath_node = nullptr;
	antlr4::tree::TerminalNode* asPath_node = nullptr;

	bool local_include = false;
	bool dynamic_linking = ctx->INCLUDE_DYNAMIC() != nullptr;

	if (ctx->KEYWORD_INCLUDE() != nullptr) {
		initial_node = ctx->KEYWORD_INCLUDE();
	} else if (ctx->KEYWORD_INCLUDE_ONCE() != nullptr) {
		initial_node = ctx->KEYWORD_INCLUDE_ONCE();
	}

	if (current_code_entity == nullptr) {
		throw_syntax_error(initial_node, "Include statements can only be used at the top level of a program");
	}

	if (ctx->SYSTEM_INCLUDE_PATH() != nullptr) {
		sourcePath_node = ctx->SYSTEM_INCLUDE_PATH();
		if (ctx->LOCAL_INCLUDE_PATH(0) != nullptr) {
			asPath_node = ctx->LOCAL_INCLUDE_PATH(0);
		}
	} else {
		sourcePath_node = ctx->LOCAL_INCLUDE_PATH(0);
		local_include = true;
		if (ctx->LOCAL_INCLUDE_PATH(1) != nullptr) {
			asPath_node = ctx->LOCAL_INCLUDE_PATH(1);
		}
	}

	// Trim the first and last characters (either quote-marks or '<' and '>') from the path
	std::string source_filename = sourcePath_node->getText();
	source_filename = source_filename.substr(1, source_filename.length() - 2);

	if (!local_include) {
		// Search for the file in the include paths
		bool found = false;
		for (const std::string& include_path : *include_paths) {
			std::string full_path = include_path + "/" + source_filename;
			if (access(full_path.c_str(), F_OK) == 0) {
				source_filename = full_path;
				found = true;
				break;
			}
		}
		if (!found) {
			throw_syntax_error(initial_node, "File not found: " + source_filename);
		}
	} else {
		// This is a "local" include -- meaning we should start scanning from the same directory as the current source file
		// Unless the path given is an absolute path (starting with '/')
		// NOT NECESSARILY the same as the current working directory
		if (source_filename[0] != '/') {
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

			// Append the source_filename to the directory
			source_filename = current_directory + "/" + source_filename;
		}
	}
	// Get the full path of the file
	char full_path[PATH_MAX];
	if (realpath(source_filename.c_str(), full_path) == nullptr) {
		throw_syntax_error(initial_node, "File not found: " + source_filename);
	}

	auto result = included_files->insert(full_path);
	if (result.second == false && ctx->KEYWORD_INCLUDE_ONCE() != nullptr) {
		// If the file was already included and this is an @include_once, skip it
		ctx->children.clear();
		return;
	}

	// Create a new listener
	BashppListener listener;
	listener.set_source_file(full_path);
	listener.set_include_paths(include_paths);
	listener.set_included(true);
	listener.set_included_from(this);
	listener.set_run_on_exit(false);
	listener.set_suppress_warnings(suppress_warnings);
	listener.set_target_bash_version(target_bash_version.first, target_bash_version.second);
	
	if (replacement_file_contents.has_value()) {
		listener.set_replacement_file_contents(replacement_file_contents->first, replacement_file_contents->second);
	}

	if (!dynamic_linking) {
		// If we're linking statically, copy the compiled code from the included file to the current program
		listener.set_code_buffer(code_buffer);
	} else {
		// Otherwise, throw its output in the garbage
		std::shared_ptr<std::ofstream> garbage_stream = std::make_shared<std::ofstream>("/dev/null");
		listener.set_code_buffer(garbage_stream);
	}
	listener.set_output_file("");

	// Create a new ANTLR input stream
	antlr4::ANTLRInputStream input;
	if (!replacement_file_contents.has_value() || replacement_file_contents->first != full_path) {
		std::ifstream file_stream(full_path);
		input = antlr4::ANTLRInputStream(file_stream);
	} else {
		// If we have replacement file contents for this file, use those instead
		input = antlr4::ANTLRInputStream(replacement_file_contents->second);
	}
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
	program->set_output_stream(code_buffer);

	// If we're linking statically, the code was also added
	// If we're linking dynamically, we need to add a little source directive here
	if (dynamic_linking) {
		if (asPath_node == nullptr) {
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
		} else {
			// If the 'as' path is given, we should use that instead
			std::string asPath = asPath_node->getText();
			asPath = asPath.substr(1, asPath.length() - 2);
			current_code_entity->add_code("source \"" + asPath + "\"\n");
		}
	}
}

void BashppListener::exitInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_syntax_errors
}
