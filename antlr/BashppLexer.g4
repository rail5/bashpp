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
	mode_singlequote,
	mode_comment,
	no_mode
};

std::stack<lexer_special_mode_type> modeStack;

#define modeStack_top (modeStack.empty() ? no_mode : modeStack.top())
#define nestedSupershellStack_top (nestedSupershellStack.empty() ? 0 : nestedSupershellStack.top())
}

ESCAPE: '\\' .;

// Supershells
SUPERSHELL_START: '@(' {
	// Per Bash conventions, no expansion takes place within a single-quoted string
	// So don't start a supershell if we're in a single-quoted string
	if (modeStack_top != mode_singlequote) {
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
	} else {
		emit(std::make_unique<CommonToken>(new CommonToken(AT_LITERAL, "@(")));
	}
};

SUPERSHELL_END: '@('; // This is a dummy token to make the lexer happy
					// The actual end of a supershell is detected by the RPAREN rule (below)

AT: '@';
DOLLAR: '$';

AT_LITERAL: '@'; // Another dummy token

// Whitespace
WS: [ \t\r]+;

// Delimiters (newlines and semicolons, as in Bash)
NEWLINE: '\n' {
	if (modeStack_top == mode_comment) {
		modeStack.pop();
	} else {
		emit(std::make_unique<CommonToken>(new CommonToken(DELIM, "\n")));
	}
};

SEMICOLON: ';' {
	emit(std::make_unique<CommonToken>(new CommonToken(DELIM, ";")));
};

DELIM: '\n'; // Another dummy token

// Comments
COMMENT: '#' {
	if (modeStack_top == mode_quote || modeStack_top == mode_singlequote || modeStack_top == mode_comment) {
		emit(std::make_unique<CommonToken>(new CommonToken(POUNDKEY, "#")));
	} else {
		modeStack.push(mode_comment);
	}
};

POUNDKEY: '#'; // Yet another dummy token

// Strings
QUOTE: '"' {
	if (modeStack_top == mode_quote) {
		emit(std::make_unique<CommonToken>(new CommonToken(QUOTE_END, "\"")));
		modeStack.pop();
	} else if (modeStack_top != mode_singlequote) {
		modeStack.push(mode_quote);
	} else {
		emit(std::make_unique<CommonToken>(new CommonToken(QUOTE_LITERAL, "\"")));
	}
};

QUOTE_END: '"'; // This is a dummy token to make the lexer happy
				// The actual end of a quote is detected by the QUOTE rule

QUOTE_LITERAL: '"'; // Another dummy token

// Single-quoted strings
//SINGLEQUOTE_STRING: '\'' ~[']* '\'';
SINGLEQUOTE: '\'' {
	if (modeStack_top == mode_singlequote) {
		emit(std::make_unique<CommonToken>(new CommonToken(SINGLEQUOTE_END, "'")));
		modeStack.pop();
	} else if (modeStack_top != mode_quote) {
		modeStack.push(mode_singlequote);
	} else {
		emit(std::make_unique<CommonToken>(new CommonToken(SINGLEQUOTE_LITERAL, "'")));
	}
};

SINGLEQUOTE_END: '\''; // This is a dummy token to make the lexer happy
						// The actual end of a singlequote is detected by the SINGLEQUOTE rule

SINGLEQUOTE_LITERAL: '\''; // Another dummy token

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
