#include <iostream>
#include "generated/parser.tab.hpp"
#include "generated/lex.yy.hpp"
#include "ModeStack.h"

yyscan_t main_lexer;

ModeStack modeStack;

extern yy::parser::symbol_type yylex();
extern void destroyLexer();

extern bool set_display_lexer_output(bool enable);

void lexOnly();

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

	modeStack.push(10);

	if (argc >= 3 && std::string(argv[2]) == "--lex") {
		set_display_lexer_output(true);
	}

	yy::parser parser;
	int result = parser.parse();

	fclose(in);
	destroyLexer();

	if (result == 0) {
		std::cout << "Parse successful.\n";
	} else {
		std::cout << "Parse failed.\n";
	}
	return result;
}

void lexOnly() {
	/**
	 * FIXME:
	 * The old ANTLR set up had the lexer run in full before the parser did anything.
	 * I.e., the lexer did a full pass through the input, producing a list of tokens
	 * which the parser then consumed **afterwards**.
	 *
	 * Flex/Bison however run the lexer and parser in tandem, with the parser requesting
	 * tokens from the lexer as needed, and the lexer responding to those requests by
	 * reading more from the input.
	 *
	 * The result is now we're able to simplify certain aspects of our extremely
	 * context-sensitive lexer by having it receive more context from the parser.
	 *
	 *		e.g.: One necessary condition for a token to be an lvalue is:
	 *			either 1) To be the first non-whitespace token in an expression
	 *			or     2) To be preceded in the expression **only** by value assignments
	 *		The old lexer had basically to do some "parsing" of its own to check
	 *		whether #2 was satisfied if #1 wasn't. Now the parser can just tell the lexer
	 *		"the last thing I parsed was an assignment." This cuts a TON of complexity.
	 *
	 * However, this also means that we can't really do a "lex only" mode properly,
	 * at least not if we want to get a 100% accurate representation of what the parser
	 * would see.
	 *
	 * If we want to keep the "lex only" mode, we'll have to do something about this.
	 * Idea #1: Some kind of "full silence" mode for the parser, where it parses
	 * but never writes anything to output.
	 *
	 * Since bpp v0.1, `-p` outputs the parse tree in a structured format,
	 * `-t` outputs the list of tokens.
	 * This has been very useful for debugging and I'm not quite ready to get rid of it.
	 * 
	 */
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
