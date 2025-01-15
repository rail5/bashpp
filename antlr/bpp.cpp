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

#include "out/BashppLexer.h"
#include "out/BashppParser.h"

#include "BashppListener.cpp"

#include "syntax_error.cpp"
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
		listener->set_source_file(argv[1]);
		walker.walk(listener, tree);

	} catch (const antlr4::EmptyStackException& e) {
		std::cerr << "EmptyStackException: " << e.what() << std::endl;
	} catch (const RecognitionException& e) {
		std::cerr << "Recognition exception: " << e.what() << std::endl;
	} catch (const syntax_error& e) {
		std::cerr << "Syntax error: " << e.what() << std::endl;
	} catch (const internal_error& e) {
		std::cerr << "Internal error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
	}

	return 0;
}