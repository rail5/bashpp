/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>
#include <AST/BashppParser.h>

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
void BashppListener::enterIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) {
	// Get the current code entity
	std::shared_ptr<bpp::bpp_program> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());

	bool local_include = node->PATHTYPE() == AST::IncludeStatement::PathType::QUOTED;
	bool dynamic_linking = node->TYPE() == AST::IncludeStatement::IncludeType::DYNAMIC;
	bool include_once = node->KEYWORD() == AST::IncludeStatement::IncludeKeyword::INCLUDE_ONCE;

	auto source_path = node->PATH();
	auto as_path = node->ASPATH();

	std::string resolved_source_path = source_path.getValue();

	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Include statements can only be used at the top level of a program");
	}

	if (!local_include) {
		// Search for the file in the include paths
		bool found = false;
		for (const std::string& include_path : *include_paths) {
			std::string full_path = include_path + "/" + source_path.getValue();
			if (access(full_path.c_str(), F_OK) == 0) {
				resolved_source_path = full_path;
				found = true;
				break;
			}
		}
		if (!found) {
			throw bpp::ErrorHandling::SyntaxError(this, source_path, "File not found: " + source_path.getValue());
		}
	} else {
		// This is a "local" include -- meaning we should start scanning from the same directory as the current source file
		// Unless the path given is an absolute path (starting with '/')
		// NOT NECESSARILY the same as the current working directory
		bool absolute_path = source_path.getValue().size() > 0 && source_path.getValue()[0] == '/';
		if (!absolute_path) {
			// Get the directory of the current source file
			// If the program is being read from stdin, we should use the current working directory
			std::string current_directory;
			if (source_file == "<stdin>") {
				char current_working_directory[PATH_MAX];
				if (getcwd(current_working_directory, PATH_MAX) == nullptr) {
					throw bpp::ErrorHandling::InternalError("Could not get current working directory");
				}
				current_directory = current_working_directory;
			} else {
				current_directory = source_file.substr(0, source_file.find_last_of('/'));
			}

			// Append the source_filename to the directory
			resolved_source_path = current_directory + "/" + source_path.getValue();
		}
	}
	// Get the full path of the file
	char full_path[PATH_MAX];
	if (realpath(resolved_source_path.c_str(), full_path) == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, source_path, "File not found: " + resolved_source_path);
	}

	auto result = included_files->insert(full_path);
	if (result.second == false && include_once) {
		// If the file was already included and this is an @include_once, skip it
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
	listener.set_target_bash_version(target_bash_version);
	for (const auto& pair : replacement_file_contents) {
		listener.set_replacement_file_contents(pair.first, pair.second);
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

	// Create a new parser
	AST::BashppParser parser; // TODO(@rail5): Propagation of UTF16 flag from the language server?
	std::vector<std::string> new_include_stack = this->include_stack;
	new_include_stack.push_back(source_file);
	parser.setIncludeChain(new_include_stack);
	
	if (replacement_file_contents.find(full_path) != replacement_file_contents.end()) {
		parser.setInputFromStringContents(replacement_file_contents[full_path]);
	} else {
		parser.setInputFromFilePath(full_path);
	}

	auto tree = parser.program();
	if (tree == nullptr) {
		auto nodeCopy = source_path;
		nodeCopy.setValue(nodeCopy.getValue() + "  "); // HACK
		// The parser removes the surrounding quote-marks or angle-brackets from the included path before passing it to us
		// And the error reporter takes the length of the error token's string to know what to highlight in its reporting to the user
		// Bad design, should just take a length to begin with
		// Until then, this hack adds two characters to the error token's string so highlighting works
		// TODO(@rail5): FIX THIS
		throw bpp::ErrorHandling::SyntaxError(this, nodeCopy, "Failed to parse included file: " + std::string(full_path));
	}
	try {
		// Walk the tree
		listener.walk(tree);
	} catch (const bpp::ErrorHandling::InternalError& e) {
		std::cerr << "Internal error from included file '" << full_path << "'" << std::endl;
		throw bpp::ErrorHandling::InternalError(e);
	} catch (const std::exception& e) {
		std::cerr << "Standard exception from included file '" << full_path << "'" << std::endl;
		throw std::exception(e);
	} catch (...) {
		std::cerr << "Unknown exception occurred (from included file '" << full_path << "')" << std::endl;
		throw;
	}

	// The objects, classes, etc should all have been added to the current program by the included program's new listener
	// Recover our original output stream
	program->set_output_stream(code_buffer);

	// If we're linking statically, the code was also added
	// If we're linking dynamically, we need to add a little source directive here
	if (dynamic_linking) {
		if (!as_path.has_value()) {
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
			current_code_entity->add_code("source \"" + as_path.value().getValue() + "\"\n");
		}
	}
}

void BashppListener::exitIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) {}
