lexer grammar BashppLexer;

@header {
#include <memory>
#include <stack>
#include <string>
}

@lexer::members {
int parenDepth = 0;
int initialSupershellDepth = 0;
int initialSubshellDepth = 0;
std::stack<int> nestedSupershellStack;
std::stack<int> nestedSubshellStack;
bool inSupershell = false;
bool inSubshell = false;
bool inDeprecatedSubshell = false;

enum lexer_special_mode_type {
	mode_supershell,
	mode_subshell,
	mode_quote,
	mode_singlequote,
	mode_comment,
	no_mode
};

std::stack<lexer_special_mode_type> modeStack;

#define modeStack_top (modeStack.empty() ? no_mode : modeStack.top())
#define nestedSupershellStack_top (nestedSupershellStack.empty() ? 0 : nestedSupershellStack.top())
#define nestedSubshellStack_top (nestedSubshellStack.empty() ? 0 : nestedSubshellStack.top())

inline bool contains_double_underscore(const std::string& s) {
	return s.find("__") != std::string::npos;
}

#define emit(tokenType, text) emit(std::make_unique<CommonToken>(new CommonToken(tokenType, text)))

}

ESCAPE: '\\' . {
	// Don't escape if we're in a single-quoted string
	if (modeStack_top == mode_singlequote) {
		if (getText() == "\\'") {
			emit(SINGLEQUOTE_END, "\\'");
			modeStack.pop();
		} else {
			emit(ESCAPE_LITERAL, getText());
		}
	}
};

ESCAPE_LITERAL: '\\' .; // Dummy token

// Supershells
SUPERSHELL_START: '@(' {
	// Per Bash conventions, no expansion takes place within a single-quoted string
	// So don't start a supershell if we're in a single-quoted string
	switch (modeStack_top) {
		case mode_singlequote:
			emit(AT_LITERAL, "@(");
			break;
		default:
			modeStack.push(mode_supershell);
			nestedSupershellStack.push(parenDepth);
			if (!inSupershell) {
				initialSupershellDepth = parenDepth;
			}
			inSupershell = true;
			parenDepth++;
			emit(SUPERSHELL_START, "@(");
			break;
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
	switch (modeStack_top) {
		case mode_comment:
			modeStack.pop();
			break;
		default:
			emit(DELIM, "\n");
			break;
	}
};

SEMICOLON: ';' {
	emit(DELIM, ";");
};

CONNECTIVE: DOUBLEAMPERSAND | DOUBLEPIPE; 
DOUBLEAMPERSAND: '&&';
DOUBLEPIPE: '||';

DELIM: '\n'; // Another dummy token

// Comments
COMMENT: '#' {
	switch (modeStack_top) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
			emit(POUNDKEY, "#");
			break;
		default:
			modeStack.push(mode_comment);
			break;
	}
};

POUNDKEY: '#'; // Yet another dummy token

// Strings
QUOTE: '"' {
	switch (modeStack_top) {
		case mode_quote:
			emit(QUOTE_END, "\"");
			modeStack.pop();
			break;
		case mode_comment:
		case mode_singlequote:
			emit(QUOTE_LITERAL, "\"");
			break;
		default:
			modeStack.push(mode_quote);
			break;
	}
};

QUOTE_END: '"'; // This is a dummy token to make the lexer happy
				// The actual end of a quote is detected by the QUOTE rule

QUOTE_LITERAL: '"'; // Another dummy token

// Single-quoted strings
SINGLEQUOTE: '\'' {
	switch (modeStack_top) {
		case mode_singlequote:
			emit(SINGLEQUOTE_END, "'");
			modeStack.pop();
			break;
		case mode_quote:
			emit(SINGLEQUOTE_LITERAL, "'");
			break;
		default:
			modeStack.push(mode_singlequote);
			break;
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
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]* {
	if (contains_double_underscore(getText())) {
		emit(INVALID_IDENTIFIER, getText());
	}
};

INVALID_IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*; // Another dummy token

// Bash variables
BASH_VAR: '$' IDENTIFIER
		| '$' '{' IDENTIFIER '}';

// Bash arithmetic
BASH_ARITH: '$' '(' '(' .+? ')' ')';

// Operators
ASSIGN: '=';
DOT: '.';

LBRACE: '{';
RBRACE: '}';

BACKTICK: '`' {
	switch (modeStack_top) {
		case mode_singlequote:
		case mode_comment:
			// Just emit the BACKTICK token
			break;
		default:
			if (!inDeprecatedSubshell) {
				emit(DEPRECATED_SUBSHELL_START, "`");
				inDeprecatedSubshell = false;
			} else {
				emit(DEPRECATED_SUBSHELL_END, "`");
				inDeprecatedSubshell = true;
			}
			break;
	}
};

DEPRECATED_SUBSHELL_START: '`'; // Another dummy token
DEPRECATED_SUBSHELL_END: '`'; // Yet another dummy token

LPAREN: '(' {
	switch (modeStack_top) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
			// Just emit the LPAREN token
			break;
		default:
			nestedSubshellStack.push(parenDepth);
			modeStack.push(mode_subshell);
			if (!inSubshell) {
				initialSubshellDepth = parenDepth;
			}
			inSubshell = true;
			emit(SUBSHELL_START, "(");
			break;
	}
	parenDepth++;
};

SUBSHELL_START: '('; // This is a dummy token to make the lexer happy
					// The actual start of a subshell is detected by the LPAREN rule (above)

RPAREN: ')' {
	parenDepth--;
	switch (modeStack_top) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
			// Just emit the RPAREN token
			break;
		case mode_subshell:
			if (parenDepth == nestedSubshellStack_top) {
				if (parenDepth == initialSubshellDepth) {
					inSubshell = false;
				}
				nestedSubshellStack.pop();
				modeStack.pop();
				emit(SUBSHELL_END, ")");
			}
			break;
		case mode_supershell:
			if (parenDepth == nestedSupershellStack_top) {
				if (parenDepth == initialSupershellDepth) {
					inSupershell = false;
				}
				nestedSupershellStack.pop();
				modeStack.pop();
				emit(SUPERSHELL_END, ")");
			}
			break;
	}
};

SUBSHELL_END: ')'; // This is a dummy token to make the lexer happy
					// The actual end of a subshell is detected by the RPAREN rule (above)

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
