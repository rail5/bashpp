lexer grammar BashppLexer;

@header {
	#include <memory>
	#include <stack>
}

@lexer::members {
int parenDepth = 0;
std::stack<int> nestedSupershellStack;
}

ESCAPE: '\\' .;

// Supershells
SUPERSHELL_START: '@(' {
	parenDepth = 1;
	pushMode(SUPERSHELL_MODE);
};

SUPERSHELL_END: '@('; // This is a dummy token to make the lexer happy
					// "SUPERSHELL_END" will be emitted when we exit the SUPERSHELL_MODE

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
STRING: DOUBLEQUOTE_STRING | SINGLEQUOTE_STRING;

// Double-quoted strings
DOUBLEQUOTE_STRING: '"' (ESCAPE . | ~["\\])* '"';

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
			| '`' .+? '`';

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

mode SUPERSHELL_MODE;
SS_ESCAPE: '\\' . { emit(std::make_unique<CommonToken>(new CommonToken(ESCAPE, getText()))); };

// Supershells
SS_SUPERSHELL_START: '@(' {
	// Add the current parenDepth to the nested supershell stack
	nestedSupershellStack.push(parenDepth);
	parenDepth++;
	emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_START, getText())));
};

SS_AT: '@' { emit(std::make_unique<CommonToken>(new CommonToken(AT, getText()))); };
SS_DOLLAR: '$' { emit(std::make_unique<CommonToken>(new CommonToken(DOLLAR, getText()))); };

// Whitespace
SS_WS: [ \t\r]+ { emit(std::make_unique<CommonToken>(new CommonToken(WS, getText()))); };

// Delimiters (newlines and semicolons, as in Bash)
SS_DELIM: SS_NEWLINE | SS_SEMICOLON { emit(std::make_unique<CommonToken>(new CommonToken(DELIM, getText()))); };

SS_NEWLINE: '\n';
SS_SEMICOLON: ';';

// Comments
SS_COMMENT: '#' ~[\n]* -> skip;

// Strings
SS_STRING: SS_DOUBLEQUOTE_STRING | SS_SINGLEQUOTE_STRING { emit(std::make_unique<CommonToken>(new CommonToken(STRING, getText()))); };

// Double-quoted strings
SS_DOUBLEQUOTE_STRING: '"' (ESCAPE . | ~["\\])* '"';

// Single-quoted strings
SS_SINGLEQUOTE_STRING: '\'' ~[']* '\'';

// Keywords
SS_KEYWORD_CLASS: 'class' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_CLASS, getText()))); };
SS_KEYWORD_PUBLIC: 'public' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_PUBLIC, getText()))); };
SS_KEYWORD_PRIVATE: 'private' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_PRIVATE, getText()))); };
SS_KEYWORD_PROTECTED: 'protected' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_PROTECTED, getText()))); };
SS_KEYWORD_VIRTUAL: 'virtual' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_VIRTUAL, getText()))); };
SS_KEYWORD_METHOD: 'method' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_METHOD, getText()))); };
SS_KEYWORD_CONSTRUCTOR: 'constructor' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_CONSTRUCTOR, getText()))); };
SS_KEYWORD_DESTRUCTOR: 'destructor' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_DESTRUCTOR, getText()))); };
SS_KEYWORD_NEW: 'new' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_NEW, getText()))); };
SS_KEYWORD_DELETE: 'delete' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_DELETE, getText()))); };
SS_KEYWORD_NULLPTR: 'nullptr' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_NULLPTR, getText()))); };
SS_KEYWORD_THIS: 'this' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_THIS, getText()))); };
SS_KEYWORD_INCLUDE: 'include' { emit(std::make_unique<CommonToken>(new CommonToken(KEYWORD_INCLUDE, getText()))); };

// Identifiers
SS_IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]* { emit(std::make_unique<CommonToken>(new CommonToken(IDENTIFIER, getText()))); };

// Bash variables
SS_BASH_VAR: '$' SS_IDENTIFIER
		| '$' '{' SS_IDENTIFIER '}' { emit(std::make_unique<CommonToken>(new CommonToken(BASH_VAR, getText()))); };

// Bash subshells
SS_BASH_SUBSHELL: '$' '(' .+? ')'
			| '`' .+? '`' { emit(std::make_unique<CommonToken>(new CommonToken(BASH_SUBSHELL, getText()))); };

// Bash arithmetic
SS_BASH_ARITH: '$' '(' '(' .+? ')' ')' { emit(std::make_unique<CommonToken>(new CommonToken(BASH_ARITH, getText()))); };

// Operators
SS_ASSIGN: '=' { emit(std::make_unique<CommonToken>(new CommonToken(ASSIGN, getText()))); };
SS_DOT: '.' { emit(std::make_unique<CommonToken>(new CommonToken(DOT, getText()))); };

SS_LBRACE: '{' { emit(std::make_unique<CommonToken>(new CommonToken(LBRACE, getText()))); };
SS_RBRACE: '}' { emit(std::make_unique<CommonToken>(new CommonToken(RBRACE, getText()))); };
SS_LPAREN: '(' { parenDepth++; emit(std::make_unique<CommonToken>(new CommonToken(LPAREN, getText()))); };
SS_RPAREN: ')' {
	parenDepth--;
	if (parenDepth == 0) {
		popMode();
		emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_END, ")")));
	} else if (parenDepth == nestedSupershellStack.top()) {
		nestedSupershellStack.pop();
		emit(std::make_unique<CommonToken>(new CommonToken(SUPERSHELL_END, getText())));
	} else {
		emit(std::make_unique<CommonToken>(new CommonToken(RPAREN, getText())));
	}
};
SS_LBRACKET: '[' { emit(std::make_unique<CommonToken>(new CommonToken(LBRACKET, getText()))); };
SS_RBRACKET: ']' { emit(std::make_unique<CommonToken>(new CommonToken(RBRACKET, getText()))); };

SS_ASTERISK: '*' { emit(std::make_unique<CommonToken>(new CommonToken(ASTERISK, getText()))); };
SS_AMPERSAND: '&' { emit(std::make_unique<CommonToken>(new CommonToken(AMPERSAND, getText()))); };

SS_NUMBER: SS_INTEGER | SS_FLOAT { emit(std::make_unique<CommonToken>(new CommonToken(NUMBER, getText()))); };

SS_INTEGER: [0-9]+;
SS_FLOAT: [0-9]+ '.' [0-9]+;

SS_MINUS: '-' { emit(std::make_unique<CommonToken>(new CommonToken(MINUS, getText()))); };
SS_PLUS: '+' { emit(std::make_unique<CommonToken>(new CommonToken(PLUS, getText()))); };
SS_SLASH: '/' { emit(std::make_unique<CommonToken>(new CommonToken(SLASH, getText()))); };
SS_EXCLAM: '!' { emit(std::make_unique<CommonToken>(new CommonToken(EXCLAM, getText()))); };
SS_COLON: ':' { emit(std::make_unique<CommonToken>(new CommonToken(COLON, getText()))); };
SS_COMMA: ',' { emit(std::make_unique<CommonToken>(new CommonToken(COMMA, getText()))); };
SS_QUESTION: '?' { emit(std::make_unique<CommonToken>(new CommonToken(QUESTION, getText()))); };
SS_TILDE: '~' { emit(std::make_unique<CommonToken>(new CommonToken(TILDE, getText()))); };
SS_CARET: '^' { emit(std::make_unique<CommonToken>(new CommonToken(CARET, getText()))); };
SS_PERCENT: '%' { emit(std::make_unique<CommonToken>(new CommonToken(PERCENT, getText()))); };
SS_LESSTHAN: '<' { emit(std::make_unique<CommonToken>(new CommonToken(LESSTHAN, getText()))); };
SS_GREATERTHAN: '>' { emit(std::make_unique<CommonToken>(new CommonToken(GREATERTHAN, getText()))); };

SS_CATCHALL: . { emit(std::make_unique<CommonToken>(new CommonToken(CATCHALL, getText()))); };
