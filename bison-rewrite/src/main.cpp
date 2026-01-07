#include <iostream>
#include "generated/parser.tab.hpp"

extern FILE* yyin;
extern int yyleng;
extern char* yytext;

void lexOnly();
yy::parser::symbol_type yylex();

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <inputfile>\n";
		return 1;
	}

	yyin = fopen(argv[1], "r");
	if (!yyin) {
		std::cerr << "Error: Could not open file " << argv[1] << "\n";
		return 1;
	}

	if (argc >= 3 && std::string(argv[2]) == "--lex") {
		lexOnly();
		return 0;
	}

	yy::parser parser;
	int result = parser.parse();

	if (result == 0) {
		std::cout << "Parse successful.\n";
	} else {
		std::cout << "Parse failed.\n";
	}
	return result;
}

void lexOnly() {
	while (true) {
		auto sym = yylex();
		if (sym.kind() == yy::parser::symbol_kind::S_YYEOF) {
			break;
		}
		std::cout << "Token: " << yy::parser::symbol_name(sym.kind());
		
		std::string semantic_value = sym.value.as<std::string>();
		if (!semantic_value.empty()) {
			std::cout << ", Text: " << semantic_value;
		}

		std::cout << std::endl;
	}
}
