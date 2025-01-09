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
	mode_quote
};

std::stack<lexer_special_mode_type> modeStack;
}

ESCAPE: '\\' .;

// Supershells
SUPERSHELL_START: '@(' {
	parenDepth = 1;
	inSupershell = true;
	pushMode(EXTRA_MODE);
	modeStack.push(mode_supershell);
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
	pushMode(EXTRA_MODE);
	modeStack.push(mode_quote);
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
RPAREN: ')';
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

mode EXTRA_MODE;
EM_ESCAPE: '\\' . { emit(std::make_unique<CommonToken>(new CommonToken(ESCAPE, getText()))); };

// Supershells
EM_SUPERSHELL_START: '@(' {
	if (inSupershell) {
		// Add the current parenDepth to the nested supershell stack
		nestedSupershellStack.push(parenDepth);
		parenDepth++;
		modeStack.push(mode_supershell);
		emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_START, getText())));
	} else {
		parenDepth = 1;
		inSupershell = true;
		pushMode(EXTRA_MODE);
		modeStack.push(mode_supershell);
		emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_START, getText())));
	}
};

EM_AT: '@' { emit(std::make_unique<CommonToken>(new CommonToken(AT, getText()))); };
EM_DOLLAR: '$' { emit(std::make_unique<CommonToken>(new CommonToken(DOLLAR, getText()))); };

// Whitespace
EM_WS: [ \t\r]+ { emit(std::make_unique<CommonToken>(new CommonToken(WS, getText()))); };

// Delimiters (newlines and semicolons, as in Bash)
EM_DELIM: EM_NEWLINE | EM_SEMICOLON { emit(std::make_unique<CommonToken>(new CommonToken(DELIM, getText()))); };

EM_NEWLINE: '\n';
EM_SEMICOLON: ';';

// Comments
EM_COMMENT: '#' ~[\n]* -> skip;

// Strings
//EM_STRING: EM_DOUBLEQUOTE_STRING | EM_SINGLEQUOTE_STRING { emit(std::make_unique<CommonToken>(new CommonToken(STRING, getText()))); };

// Double-quoted strings
//EM_DOUBLEQUOTE_STRING: '"' (ESCAPE . | ~["\\])* '"';

EM_QUOTE: '"' {
	if (modeStack.top() == mode_quote) {
		emit(std::make_unique<CommonToken>(new CommonToken(QUOTE_END, "\"")));
		popMode();
		modeStack.pop();
	} else {
		pushMode(EXTRA_MODE);
		modeStack.push(mode_quote);
		emit(std::make_unique<CommonToken>(new CommonToken(QUOTE, "\"")));
	}
};

// Single-quoted strings
EM_SINGLEQUOTE_STRING: '\'' ~[']* '\'';

// Keywords
EM_KEYWORD_CLASS: 'class' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_CLASS, getText()))); };
EM_KEYWORD_PUBLIC: 'public' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_PUBLIC, getText()))); };
EM_KEYWORD_PRIVATE: 'private' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_PRIVATE, getText()))); };
EM_KEYWORD_PROTECTED: 'protected' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_PROTECTED, getText()))); };
EM_KEYWORD_VIRTUAL: 'virtual' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_VIRTUAL, getText()))); };
EM_KEYWORD_METHOD: 'method' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_METHOD, getText()))); };
EM_KEYWORD_CONSTRUCTOR: 'constructor' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_CONSTRUCTOR, getText()))); };
EM_KEYWORD_DESTRUCTOR: 'destructor' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_DESTRUCTOR, getText()))); };
EM_KEYWORD_NEW: 'new' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_NEW, getText()))); };
EM_KEYWORD_DELETE: 'delete' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_DELETE, getText()))); };
EM_KEYWORD_NULLPTR: 'nullptr' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_NULLPTR, getText()))); };
EM_KEYWORD_THIS: 'this' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_THIS, getText()))); };
EM_KEYWORD_INCLUDE: 'include' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_INCLUDE, getText()))); };

// Identifiers
EM_IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]* { emit(std::make_unique<CommonToken>(new CommonToken(IDENTIFIER, getText()))); };

// Bash variables
EM_BASH_VAR: '$' EM_IDENTIFIER
		| '$' '{' EM_IDENTIFIER '}' { emit(std::make_unique<CommonToken>(new CommonToken(BASH_VAR, getText()))); };

// Bash subshells
EM_BASH_SUBSHELL: '$' '(' .+? ')'
			| '`' '`' (ESCAPE . | ~[`\\])* '`' '`' { emit(std::make_unique<CommonToken>(new CommonToken(BASH_SUBSHELL, getText()))); };

// Bash arithmetic
EM_BASH_ARITH: '$' '(' '(' .+? ')' ')' { emit(std::make_unique<CommonToken>(new CommonToken(BASH_ARITH, getText()))); };

// Operators
EM_ASSIGN: '=' { emit(std::make_unique<CommonToken>(new CommonToken(ASSIGN, getText()))); };
EM_DOT: '.' { emit(std::make_unique<CommonToken>(new CommonToken(DOT, getText()))); };

EM_LBRACE: '{' { emit(std::make_unique<CommonToken>(new CommonToken(LBRACE, getText()))); };
EM_RBRACE: '}' { emit(std::make_unique<CommonToken>(new CommonToken(RBRACE, getText()))); };
EM_LPAREN: '(' { parenDepth++; emit(std::make_unique<CommonToken>(new CommonToken(LPAREN, getText()))); };
EM_RPAREN: ')' {
	if (inSupershell && modeStack.top() == mode_supershell) {
		parenDepth--;
		if (parenDepth == 0) {
			inSupershell = false;
			popMode();
			modeStack.pop();
			emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_END, ")")));
		} else if (parenDepth == nestedSupershellStack.top()) {
			nestedSupershellStack.pop();
			modeStack.pop();
			emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_END, ")")));
		} else {
			emit(std::make_unique<CommonToken>(new CommonToken(RPAREN, ")")));
		}
	} else {
		emit(std::make_unique<CommonToken>(new CommonToken(RPAREN, ")")));
	}
};
EM_LBRACKET: '[' { emit(std::make_unique<CommonToken>(new CommonToken(LBRACKET, getText()))); };
EM_RBRACKET: ']' { emit(std::make_unique<CommonToken>(new CommonToken(RBRACKET, getText()))); };

EM_ASTERISK: '*' { emit(std::make_unique<CommonToken>(new CommonToken(ASTERISK, getText()))); };
EM_AMPERSAND: '&' { emit(std::make_unique<CommonToken>(new CommonToken(AMPERSAND, getText()))); };

EM_NUMBER: EM_INTEGER | EM_FLOAT { emit(std::make_unique<CommonToken>(new CommonToken(NUMBER, getText()))); };

EM_INTEGER: [0-9]+;
EM_FLOAT: [0-9]+ '.' [0-9]+;

EM_MINUS: '-' { emit(std::make_unique<CommonToken>(new CommonToken(MINUS, getText()))); };
EM_PLUS: '+' { emit(std::make_unique<CommonToken>(new CommonToken(PLUS, getText()))); };
EM_SLASH: '/' { emit(std::make_unique<CommonToken>(new CommonToken(SLASH, getText()))); };
EM_EXCLAM: '!' { emit(std::make_unique<CommonToken>(new CommonToken(EXCLAM, getText()))); };
EM_COLON: ':' { emit(std::make_unique<CommonToken>(new CommonToken(COLON, getText()))); };
EM_COMMA: ',' { emit(std::make_unique<CommonToken>(new CommonToken(COMMA, getText()))); };
EM_QUESTION: '?' { emit(std::make_unique<CommonToken>(new CommonToken(QUESTION, getText()))); };
EM_TILDE: '~' { emit(std::make_unique<CommonToken>(new CommonToken(TILDE, getText()))); };
EM_CARET: '^' { emit(std::make_unique<CommonToken>(new CommonToken(CARET, getText()))); };
EM_PERCENT: '%' { emit(std::make_unique<CommonToken>(new CommonToken(PERCENT, getText()))); };
EM_LESSTHAN: '<' { emit(std::make_unique<CommonToken>(new CommonToken(LESSTHAN, getText()))); };
EM_GREATERTHAN: '>' { emit(std::make_unique<CommonToken>(new CommonToken(GREATERTHAN, getText()))); };

EM_CATCHALL: . { emit(std::make_unique<CommonToken>(new CommonToken(CATCHALL, getText()))); };
