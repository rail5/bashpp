lexer grammar BashppLexer;

@header {
#include <memory>
#include <stack>
}

@lexer::members {
int parenDepth = 0;
std::stack<int> nestedSupershellStack;
bool inSupershell = false;

enum lexer_special_mode_type {
	mode_supershell,
	mode_quote,
	no_mode
};

std::stack<lexer_special_mode_type> modeStack;

#define modeStack_top (modeStack.empty() ? no_mode : modeStack.top())
#define nestedSupershellStack_top (nestedSupershellStack.empty() ? 0 : nestedSupershellStack.top())
}

ESCAPE: '\\' .;

// Supershells
SUPERSHELL_START: '@(' {
	if (inSupershell) {
		// Add the current parenDepth to the nested supershell stack
		nestedSupershellStack.push(parenDepth);
		parenDepth++;
		modeStack.push(mode_supershell);
	} else {
		parenDepth = 1;
		inSupershell = true;
		modeStack.push(mode_supershell);
	}
};

SUPERSHELL_END: '@('; // This is a dummy token to make the lexer happy
					// "SUPERSHELL_END" will be emitted when we exit the EXTRA_MODE

AT: '@';
DOLLAR: '$';

// Whitespace
WS: [ \t\r]+;

// Delimiters (newlines and semicolons, as in Bash)
DELIM: NEWLINE | SEMICOLON;

NEWLINE: '\n';
SEMICOLON: ';';

// Comments
COMMENT: '#' ~[\n]* -> skip;

// Strings
//STRING: DOUBLEQUOTE_STRING | SINGLEQUOTE_STRING;

// Double-quoted strings
//DOUBLEQUOTE_STRING: '"' (ESCAPE . | ~["\\])* '"';

QUOTE: '"' {
	std::cout << "Encountered quote" << std::endl;
	if (modeStack_top == mode_quote) {
		emit(std::make_unique<CommonToken>(new CommonToken(QUOTE_END, "\"")));
		modeStack.pop();
	} else {
		modeStack.push(mode_quote);
	}
};

QUOTE_END: '"'; // This is a dummy token to make the lexer happy
				// "QUOTE_END" will be emitted when we exit the EXTRA_MODE

// Single-quoted strings
SINGLEQUOTE_STRING: '\'' ~[']* '\'';

// Keywords
KEYWORD_CLASS: 'class';
KEYWORD_PUBLIC: 'public';
KEYWORD_PRIVATE: 'private';
KEYWORD_PROTECTED: 'protected';
KEYWORD_VIRTUAL: 'virtual';
KEYWORD_METHOD: 'method';
KEYWORD_CONSTRUCTOR: 'constructor';
KEYWORD_DESTRUCTOR: 'destructor';
KEYWORD_NEW: 'new';
KEYWORD_DELETE: 'delete';
KEYWORD_NULLPTR: 'nullptr';
KEYWORD_THIS: 'this';
KEYWORD_INCLUDE: 'include';

// Identifiers
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;

// Bash variables
BASH_VAR: '$' IDENTIFIER
		| '$' '{' IDENTIFIER '}';

// Bash subshells
BASH_SUBSHELL: '$' '(' .+? ')'
			| '`' (ESCAPE . | ~[`\\])* '`';

// Bash arithmetic
BASH_ARITH: '$' '(' '(' .+? ')' ')';

// Operators
ASSIGN: '=';
DOT: '.';

LBRACE: '{';
RBRACE: '}';
LPAREN: '(';
RPAREN: ')' {
	std::cout << "Encountered RPAREN" << std::endl;
	if (inSupershell && modeStack_top == mode_supershell) {
		parenDepth--;
		if (parenDepth == 0) {
			inSupershell = false;
			modeStack.pop();
			emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_END, ")")));
		} else if (parenDepth == nestedSupershellStack_top) {
			nestedSupershellStack.pop();
			modeStack.pop();
			emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_END, ")")));
		}
	}
};
LBRACKET: '[';
RBRACKET: ']';

ASTERISK: '*';
AMPERSAND: '&';

NUMBER: INTEGER | FLOAT;

INTEGER: [0-9]+;
FLOAT: [0-9]+ '.' [0-9]+;

MINUS: '-';
PLUS: '+';
SLASH: '/';
EXCLAM: '!';
COLON: ':';
COMMA: ',';
QUESTION: '?';
TILDE: '~';
CARET: '^';
PERCENT: '%';
LESSTHAN: '<';
GREATERTHAN: '>';

CATCHALL: .;
