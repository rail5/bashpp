/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <iostream>
#include "generated/parser.tab.hpp"
#include "generated/lex.yy.hpp"
#include "ModeStack.h"
#include "AST/Listener/BaseListener.h"

yyscan_t main_lexer;

ModeStack modeStack;

extern yy::parser::symbol_type yylex();
extern void initLexer();
extern void destroyLexer();

extern bool set_display_lexer_output(bool enable);

class BashppListener : public AST::BaseListener {
	public:
		void enterProgram(std::shared_ptr<AST::Program> node) override {
			std::cout << "Entering Program Node\n";
		}

		void exitProgram(std::shared_ptr<AST::Program> node) override {
			std::cout << "Exiting Program Node\n";
		}

		void enterIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) override {
			std::cout << "Entering Include Statement Node\n";
		}

		void exitIncludeStatement(std::shared_ptr<AST::IncludeStatement> node) override {
			std::cout << "Exiting Include Statement Node\n";
		}
};

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <inputfile>\n";
		return 1;
	}

	if (yylex_init(&main_lexer) != 0) {
		std::cerr << "Error: Could not initialize lexer\n";
		return 1;
	}

	modeStack.bind(main_lexer);

	FILE* in = fopen(argv[1], "r");
	if (!in) {
		std::cerr << "Error: Could not open file " << argv[1] << "\n";
		destroyLexer();
		return 1;
	}

	yyset_in(in, main_lexer);

	initLexer();

	if (argc >= 3 && std::string(argv[2]) == "--lex") {
		set_display_lexer_output(true);
	}
	
	std::shared_ptr<AST::Program> program;

	yy::parser parser(program);
	int result = parser.parse();

	BashppListener listener;
	listener.walk(program);

	fclose(in);
	destroyLexer();

	if (result == 0) {
		std::cout << "Parse successful.\n";
	} else {
		std::cout << "Parse failed.\n";
	}
	return result;
}
