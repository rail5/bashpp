#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <antlr4-runtime.h>

#include "out/BashppLexer.h"
#include "out/BashppParser.h"

#include "bpp_class.cpp"
#include "syntax_error.cpp"

using namespace antlr4;

std::set<std::string> protected_keywords = {  "public", "private", "class", "method", "constructor", "primitive" };
std::set<std::string> used_symbols;
std::map<std::string, bpp_class> classes;
std::map<std::string, bpp_object> objects;
std::map<std::string, int> line_numbers;

int brace_depth = 0;

#define SYMBOL_SEPARATOR "_"

std::string generate_symbol(bool in_class, std::string class_name,
		bool in_method, std::string method_name,
		bool in_constructor,
		std::vector<bpp_class_variable> parameters = {},
		bool is_variable = false, std::string variable_name = "") {

	std::string symbol = "bpp_";

	#define MAYBE_SYMBOL_SEPARATOR (!symbol.empty() ? SYMBOL_SEPARATOR : "")
	
	if (in_class) {
		symbol += class_name;
	}

	if (in_method) {
		symbol += MAYBE_SYMBOL_SEPARATOR;
		symbol += method_name;
	}

	if (in_constructor) {
		symbol += MAYBE_SYMBOL_SEPARATOR;
		symbol += "constructor";
	}

	if (!parameters.empty()) {
		symbol += MAYBE_SYMBOL_SEPARATOR;
		for (auto& parameter : parameters) {
			symbol += MAYBE_SYMBOL_SEPARATOR;
			symbol += parameter.variable_type;
		}
	}

	if (is_variable) {
		if (in_class) {
			symbol += MAYBE_SYMBOL_SEPARATOR;
			symbol += "${objectName}";
		}
		symbol += MAYBE_SYMBOL_SEPARATOR;
		symbol += variable_name;
	}

	return symbol;
}

std::string generate_method_signature(ParserRuleContext* ruleContext, bool is_constructor = false) {
	// The ruleContext is the method_definition or constructor_definition rule
	std::string signature;
	if (is_constructor) {
		signature += "constructor";
	} else {
		signature += ruleContext->children[2]->getText();
	}

	// Iterate over the ruleContext until we find the parameter list
	for (size_t i = 0; i < ruleContext->children.size(); i++) {
		auto innerRuleContext = dynamic_cast<ParserRuleContext*>(ruleContext->children[i]);
		if (innerRuleContext) {
			if (innerRuleContext->getRuleIndex() == BashppParser::RuleParameter_list) {
				// Found the parameter list
				signature += SYMBOL_SEPARATOR;
				bool type_set_without_name = false;
				for (size_t j = 0; j < innerRuleContext->children.size(); j++) {
					auto terminalNode = dynamic_cast<tree::TerminalNode*>(innerRuleContext->children[j]);
					if (terminalNode) {
						// Is this token a BASHPP_VARIABLE? Or an IDENTIFIER?
						// We only care about the type
						switch (terminalNode->getSymbol()->getType()) {
							case BashppLexer::BASHPP_VARIABLE:
								signature += SYMBOL_SEPARATOR;
								signature += terminalNode->getText().substr(1, terminalNode->getText().size() - 1); // Strip the @ symbol
								type_set_without_name = true;
								break;
							case BashppLexer::IDENTIFIER:
								if (type_set_without_name) {
									// Name found, not primitive, move along
									type_set_without_name = false;
								} else {
									// Primitive type
									signature += SYMBOL_SEPARATOR;
									signature += "primitive";
								}
								break;
							default:
								break;
						}
					}
				}
				break;
			}
		}
	}

	return signature;
}

std::string build_program(tree::ParseTree* tree,
		bool in_class = false, std::string class_name = "",
		bool in_method = false, std::string method_signature = "",
		bool in_constructor = false,
		std::string source_file = "") {
	if (tree == nullptr) {
		return "";
	}

	if (line_numbers.find(source_file) == line_numbers.end()) {
		line_numbers[source_file] = 1;
	}

	std::string program;
	for (size_t i = 0; i < tree->children.size(); i++) {
		auto ruleContext = dynamic_cast<ParserRuleContext*>(tree->children[i]);
		if (ruleContext) {
			switch (ruleContext->getRuleIndex()) {
				case BashppParser::RuleProgram:
					program += build_program(tree->children[i], in_class, class_name, in_method, method_signature, in_constructor, source_file);
					break;
				case BashppParser::RuleStatement:
					std::cout << "Descending into statement: " << tree->children[i]->getText() << std::endl;
					program += build_program(tree->children[i], in_class, class_name, in_method, method_signature, in_constructor, source_file);
					break;
				case BashppParser::RuleClass_definition: {
					if (in_class) {
						throw syntax_error("Cannot define a class within a class", source_file, line_numbers[source_file]);
					}

					class_name = ruleContext->children[1]->getText();

					std::string class_symbol = generate_symbol(true, class_name, in_method, method_signature, in_constructor);
					std::cout << "Generated symbol for class: " << class_symbol << std::endl;

					if (used_symbols.find(class_symbol) != used_symbols.end()) {
						throw syntax_error("Symbol '" + class_symbol + "' already in use", source_file, line_numbers[source_file]);
					}

					used_symbols.insert(class_symbol);

					if (protected_keywords.find(class_name) != protected_keywords.end()) {
						throw syntax_error("Cannot use keyword '" + class_name + "' as a class name", source_file, line_numbers[source_file]);
					}

					classes[class_name] = bpp_class();
					std::cout << "Class definition: " << class_name << std::endl;
					in_class = true;
					program += build_program(tree->children[i], in_class, class_name, in_method, method_signature, in_constructor, source_file);
					in_class = false;
					break; }
				case BashppParser::RuleClass_body_statement: {
					if (!in_class) {
						throw syntax_error("Cannot define a class body statement outside of a class", source_file, line_numbers[source_file]);
					}
					std::cout << "Trying to parse class_body_statement: " << ruleContext->children[0]->getText() << std::endl;
					auto innerRuleContext = dynamic_cast<ParserRuleContext*>(ruleContext->children[0]);
					if (innerRuleContext) {
						switch (innerRuleContext->getRuleIndex()) {
							case BashppParser::RuleVariable_definition: {
								std::cout << "Variable definition: " << innerRuleContext->children[0]->getText() << std::endl;
								size_t number_of_tokens = innerRuleContext->children.size();
								std::string variable_name, variable_type, variable_scope;
								variable_scope = innerRuleContext->children[0]->getText();
								if (number_of_tokens == 3) {
									variable_name = innerRuleContext->children[2]->getText();
									variable_type = innerRuleContext->children[1]->getText();
								} else {
									variable_name = innerRuleContext->children[1]->getText();
									variable_type = ""; // Primitive type
								}

								if (protected_keywords.find(variable_name) != protected_keywords.end()) {
									throw syntax_error("Cannot use keyword '" + variable_name + "' as a variable name", source_file, line_numbers[source_file]);
								}

								if (classes[class_name].variables.find(variable_name) != classes[class_name].variables.end()) {
									throw syntax_error("Variable '" + variable_name + "' already defined in class '" + class_name + "'", source_file, line_numbers[source_file]);
								}

								std::string variable_symbol = generate_symbol(in_class, class_name, in_method, method_signature, in_constructor, {}, true, variable_name);

								if (used_symbols.find(variable_symbol) != used_symbols.end()) {
									throw syntax_error("Symbol '" + variable_symbol + "' already in use", source_file, line_numbers[source_file]);
								}

								std::cout << "Generated symbol for variable: " << variable_symbol << std::endl;

								used_symbols.insert(variable_symbol);

								classes[class_name].variables[variable_name] = bpp_class_variable();
								classes[class_name].variables[variable_name].variable_type = variable_type;
								classes[class_name].variables[variable_name].variable_name = variable_name;
								classes[class_name].variables[variable_name].variable_scope = variable_scope;
								std::cout << "Declared type: " << classes[class_name].variables[variable_name].variable_type << std::endl;
								std::cout << "Declared scope: " << classes[class_name].variables[variable_name].variable_scope << std::endl;
								std::cout << "Declared name: " << classes[class_name].variables[variable_name].variable_name << std::endl;
								break; }
							case BashppParser::RuleMethod_definition: {
								std::cout << "Method definition: " << innerRuleContext->children[0]->getText() << std::endl;
								in_method = true;
								std::string method_name = innerRuleContext->children[2]->getText();

								if (protected_keywords.find(method_name) != protected_keywords.end()) {
									throw syntax_error("Cannot use keyword '" + method_name + "' as a method name", source_file, line_numbers[source_file]);
								}

								method_signature = generate_method_signature(innerRuleContext);
								std::cout << "Generated method signature: " << method_signature << std::endl;

								if (classes[class_name].methods.find(method_signature) != classes[class_name].methods.end()) {
									throw syntax_error("Method signature '" + method_signature + "' already defined in class '" + class_name + "'", source_file, line_numbers[source_file]);
								}

								std::string method_symbol = generate_symbol(in_class, class_name, in_method, method_signature, in_constructor);
								std::cout << "Generated method symbol: " << method_symbol << std::endl;
								if (used_symbols.find(method_symbol) != used_symbols.end()) {
									throw syntax_error("Symbol '" + method_symbol + "' already in use", source_file, line_numbers[source_file]);
								}
								used_symbols.insert(method_symbol);

								classes[class_name].methods[method_signature] = bpp_class_method();
								classes[class_name].methods[method_signature].method_name = method_name;
								classes[class_name].methods[method_signature].method_scope = innerRuleContext->children[0]->getText();
								std::cout << "Declared scope: " << classes[class_name].methods[method_signature].method_scope << std::endl;
								std::cout << "Declared name: " << classes[class_name].methods[method_signature].method_name << std::endl;
								program += build_program(innerRuleContext, in_class, class_name, in_method, method_name, in_constructor, source_file);
								in_method = false;
								break; }
							case BashppParser::RuleConstructor_definition: {
								std::cout << "Constructor definition: " << innerRuleContext->children[0]->getText() << std::endl;
								in_constructor = true;
								std::string constructor_signature = generate_method_signature(innerRuleContext, true);
								std::cout << "Generated constructor signature: " << constructor_signature << std::endl;

								if (classes[class_name].constructors.size() > 0) {
									throw syntax_error("Constructor already defined for class '" + class_name + "'", source_file, line_numbers[source_file]);
								}

								classes[class_name].constructors.push_back(bpp_class_constructor());
								std::string constructor_symbol = generate_symbol(in_class, class_name, in_method, constructor_signature, in_constructor);
								std::cout << "Generated constructor symbol: " << constructor_symbol << std::endl;

								if (used_symbols.find(constructor_symbol) != used_symbols.end()) {
									throw syntax_error("Symbol '" + constructor_symbol + "' already in use", source_file, line_numbers[source_file]);
								}

								used_symbols.insert(constructor_symbol);

								build_program(innerRuleContext, in_class, class_name, in_method, constructor_signature, in_constructor, source_file);
								in_constructor = false;
								break; }
							default:
								program += "Not identified: " + std::to_string(innerRuleContext->getRuleIndex()) + "\n";
								break;
						}
					}
					break;
				}
				case BashppParser::RuleParameter_list:
					if (!in_method && !in_constructor) {
						throw syntax_error("Cannot define a parameter list outside of a method or constructor", source_file, line_numbers[source_file]);
					}
					std::cout << "Parameter list: " << ruleContext->getText() << std::endl;
					// Iterate over each token in the parameter list
					for (size_t i = 0; i < ruleContext->children.size(); i++) {
						// Is this token a BASHPP_VARIABLE? Or an IDENTIFIER?
						auto terminalNode = dynamic_cast<tree::TerminalNode*>(ruleContext->children[i]);
						if (terminalNode) {
							switch (terminalNode->getSymbol()->getType()) {
								case BashppLexer::BASHPP_VARIABLE:
									std::cout << "BASHPP_VARIABLE: " << terminalNode->getText() << std::endl;
									classes[class_name].methods[method_signature].method_parameters.push_back(bpp_class_variable());
									classes[class_name].methods[method_signature].method_parameters.back().variable_type = terminalNode->getText();
									break;
								case BashppLexer::IDENTIFIER:
									std::cout << "IDENTIFIER: " << terminalNode->getText() << std::endl;
									if (classes[class_name].methods[method_signature].method_parameters.size() > 0) {
										// Does the last-pushed variable have a name?
										if (classes[class_name].methods[method_signature].method_parameters.back().variable_name.empty()) {
											// No name -- that means we pushed a type, and this is the name
											classes[class_name].methods[method_signature].method_parameters.back().variable_name = terminalNode->getText();
											classes[class_name].methods[method_signature].method_parameters.back().variable_scope = SCOPE_UNDEFINED;
										} else {
											// There was a name. That means this is a new variable, and it's a primitive type
											classes[class_name].methods[method_signature].method_parameters.push_back(bpp_class_variable());
											classes[class_name].methods[method_signature].method_parameters.back().variable_type = "";
											classes[class_name].methods[method_signature].method_parameters.back().variable_name = terminalNode->getText();
											classes[class_name].methods[method_signature].method_parameters.back().variable_scope = SCOPE_UNDEFINED;
										}
									} else {
										// No parameters yet, so this is a primitive type
										classes[class_name].methods[method_signature].method_parameters.push_back(bpp_class_variable());
										classes[class_name].methods[method_signature].method_parameters.back().variable_type = "";
										classes[class_name].methods[method_signature].method_parameters.back().variable_name = terminalNode->getText();
										classes[class_name].methods[method_signature].method_parameters.back().variable_scope = SCOPE_UNDEFINED;
									}

									std::cout << "Declared type: " << classes[class_name].methods[method_signature].method_parameters.back().variable_type << std::endl;
									std::cout << "Declared scope: " << classes[class_name].methods[method_signature].method_parameters.back().variable_scope << std::endl;
									std::cout << "Declared name: " << classes[class_name].methods[method_signature].method_parameters.back().variable_name << std::endl;

									break;
								default:
									std::cout << "Not identified: " << terminalNode->getSymbol()->getType() << std::endl;
									break;
							}
						}
					}
					break;
				case BashppParser::RuleOther_statement:
					std::cout << "Other statement: " << ruleContext->getText() << std::endl;
					#define MAYBE_TOKEN_SEPARATOR (ruleContext->getText() != "\n" ? " " : "")
					if (in_method) {
						std::cout << "Added statement to method body for '" << method_signature << "'" << std::endl;
						classes[class_name].methods[method_signature].method_body += ruleContext->getText() + MAYBE_TOKEN_SEPARATOR;
					} else if (in_constructor) {
						classes[class_name].constructors.back().constructor_body += ruleContext->getText() + MAYBE_TOKEN_SEPARATOR;
					} else {
						program += ruleContext->getText() + MAYBE_TOKEN_SEPARATOR;
					}
					// If this is a newline, increment the line number
					if (ruleContext->getText() == "\n") {
						line_numbers[source_file]++;
					}
					break;
				case BashppParser::RuleObject_reference:
					std::cout << "Object reference: " << ruleContext->getText() << std::endl;
					break;
				case BashppParser::RuleObject_assignment:
					std::cout << "Object assignment: " << ruleContext->getText() << std::endl;
					break;
				case BashppParser::RuleObject_instantiation: {
					std::cout << "Object instantiation: " << ruleContext->getText() << std::endl;
					std::string object_type = ruleContext->children[0]->getText().substr(1, ruleContext->children[0]->getText().size() - 1);
					std::string object_name = ruleContext->children[1]->getText();
					std::string object_symbol = generate_symbol(in_class, class_name, in_method, method_signature, in_constructor, {}, true, object_name);

					std::cout << "Generated object symbol: " << object_symbol << std::endl;

					if (protected_keywords.find(object_name) != protected_keywords.end()) {
						throw syntax_error("Cannot use keyword '" + object_name + "' as an object name", source_file, line_numbers[source_file]);
					}

					if (used_symbols.find(object_symbol) != used_symbols.end()) {
						throw syntax_error("Symbol '" + object_symbol + "' already in use", source_file, line_numbers[source_file]);
					}

					if (classes.find(object_type) == classes.end()) {
						throw syntax_error("Class '" + object_type + "' not defined", source_file, line_numbers[source_file]);
					}

					if (in_class) {
						if (class_name == object_type) {
							throw syntax_error("Cannot instantiate object of same type as containing class", source_file, line_numbers[source_file]);
						}
					}

					used_symbols.insert(object_symbol);
					objects.insert({ object_symbol, bpp_object() });

					objects[object_symbol].object_name = object_name;
					objects[object_symbol].object_type = &classes[object_type];
					objects[object_symbol].in_method = in_method || in_constructor;
					objects[object_symbol].containing_class = class_name;
					objects[object_symbol].method_signature = method_signature;
					objects[object_symbol].object_symbol = object_symbol;

					std::cout << "Object type: " << object_type << std::endl;
					std::cout << "Object name: " << object_name << std::endl;
					std::cout << "Contained inside a method or constructor? " << (objects[object_symbol].in_method ? "Yes" : "No") << std::endl;
					std::cout << "Containing class: " << objects[object_symbol].containing_class << std::endl;
					std::cout << "Current class: " << class_name << std::endl;
					std::cout << "Method signature: " << objects[object_symbol].method_signature << std::endl;
					std::cout << "Object symbol: " << objects[object_symbol].object_symbol << std::endl;

					std::string instantiation_code = "";

					// Call the constructor
					if (!objects[object_symbol].object_type->constructors.empty()) {
						std::cout << "Calling constructor for object type '" << object_type << "'" << std::endl;
						instantiation_code += objects[object_symbol].object_type->constructors[0].constructor_body;
						std::cout << "Constructor contents: " << instantiation_code << std::endl;
					} else {
						std::cout << "No constructor defined for object type '" << object_type << "'" << std::endl;
					}

					program += instantiation_code;
					break; }
				case BashppParser::RuleOpen_brace:
					brace_depth++;
					break;
				case BashppParser::RuleClose_brace:
					brace_depth--;
					if (brace_depth == 0) {
						class_name = "";
						in_class = false;
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
		std::string program = build_program(tree, false, "", false, "", false, argv[1]);
		std::cout << "Program: " << std::endl;
		std::cout << program << std::endl;
	} catch (const antlr4::EmptyStackException& e) {
		std::cerr << "EmptyStackException: " << e.what() << std::endl;
	} catch (const RecognitionException& e) {
		std::cerr << "Recognition exception: " << e.what() << std::endl;
	} catch (const syntax_error& e) {
		std::cerr << "Syntax error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Unknown exception occurred" << std::endl;
	}

	return 0;
}