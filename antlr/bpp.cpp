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

int main(int argc, const char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
		return 1;
	}

	std::ifstream stream(argv[1]);
	if (!stream) {
		std::cerr << "Error: Could not open file " << argv[1] << std::endl;
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
	parser.addErrorListener(new DiagnosticErrorListener());

	// Enable the parser to use diagnostic messages
	parser.setErrorHandler(std::make_shared<BailErrorStrategy>());

	tree::ParseTree* tree = nullptr;
	try {
		tree = parser.program();
		// Walk the tree
		antlr4::tree::ParseTreeWalker walker;
		BashppListener* listener = new BashppListener();
		char full_path[PATH_MAX];
		if (realpath(argv[1], full_path) == nullptr) {
			std::cerr << "Error: Could not resolve full path for file " << argv[1] << std::endl;
			return 1;
		}
		listener->set_source_file(full_path);
		walker.walk(listener, tree);

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