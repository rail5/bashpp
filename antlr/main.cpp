#include <iostream>
#include <fstream>
#include <map>
#include <antlr4-runtime.h>

#include "out/BashppLexer.h"
#include "out/BashppParser.h"

#include "bpp_class.cpp"

using namespace antlr4;

std::map<std::string, bpp_class> classes;
std::string current_class = "";
int brace_depth = 0;

std::string build_program(tree::ParseTree* tree) {
	std::string program;
	for (size_t i = 0; i < tree->children.size(); i++) {
		auto ruleContext = dynamic_cast<ParserRuleContext*>(tree->children[i]);
		if (ruleContext) {
			switch (ruleContext->getRuleIndex()) {
				case BashppParser::RuleProgram:
					program += "#!/usr/bin/env bash\n";
					program += build_program(tree->children[i]);
					break;
				case BashppParser::RuleStatement:
					std::cout << "Descending into statement: " << tree->children[i]->getText() << std::endl;
					program += build_program(tree->children[i]);
					break;
				case BashppParser::RuleClass_definition:
					current_class = ruleContext->children[1]->getText();
					classes[current_class] = bpp_class();
					std::cout << "Class definition: " << current_class << std::endl;
					break;
				case BashppParser::RuleClass_body_statement: {
					auto innerRuleContext = dynamic_cast<ParserRuleContext*>(ruleContext->children[1]);
					if (innerRuleContext) {
						switch (innerRuleContext->getRuleIndex()) {
							case BashppParser::RuleVariable_definition:
								std::cout << "Variable definition: " << innerRuleContext->children[0]->getText() << std::endl;
								break;
							case BashppParser::RuleMethod_definition:
								std::cout << "Method definition: " << innerRuleContext->children[0]->getText() << std::endl;
								break;
							case BashppParser::RuleConstructor_definition:
								std::cout << "Constructor definition: " << innerRuleContext->children[0]->getText() << std::endl;
								break;
							default:
								program += "Not identified: " + std::to_string(innerRuleContext->getRuleIndex()) + "\n";
								break;
						}
					}
					break;
				}
				case BashppParser::RuleOpen_brace:
					brace_depth++;
					break;
				case BashppParser::RuleClose_brace:
					brace_depth--;
					if (brace_depth == 0) {
						current_class = "";
					}
					break;
				default:
					program += "Not identified: " + std::to_string(ruleContext->getRuleIndex()) + "\n";
					break;
			}
		}
	}

	return program;
}

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

	// Print tokens and their types for debugging
	tokens.fill();
	for (auto token : tokens.getTokens()) {
		std::cout << "Token: " << token->getText() 
				  << ", Type: " << lexer.getVocabulary().getSymbolicName(token->getType()) 
				  << std::endl;
	}

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
		std::cout << "Parse tree: " << std::endl;
		std::cout << tree->toStringTree(&parser, true) << std::endl;
		std::string program = build_program(tree);
		std::cout << "Program: " << std::endl;
		std::cout << program << std::endl;
	} catch (const antlr4::EmptyStackException& e) {
		std::cerr << "EmptyStackException: " << e.what() << std::endl;
	} catch (const RecognitionException& e) {
		std::cerr << "Recognition exception: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
	}

	return 0;
}