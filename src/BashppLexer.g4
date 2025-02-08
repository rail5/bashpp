lexer grammar BashppLexer;

@header {
#include <memory>
#include <string>
#include <cstdint>

#include "../SensibleStack.cpp"
}

@lexer::members {
int parenDepth = 0;
int braceDepth = 0;
int initialSupershellDepth = 0;
int initialSubshellDepth = 0;
int IfDepth = 0;
int CaseDepth = 0;
int64_t most_recent_case_keyword_metatoken_position = INT64_MIN;
SensibleStack<int> nestedSupershellStack;
SensibleStack<int> nestedSubshellStack;
SensibleStack<int> nestedArithStack;
bool inSupershell = false;
bool inSubshell = false;
bool inDeprecatedSubshell = false;

int64_t metaTokenCount = 0;

bool incoming_token_can_be_lvalue = true;
bool hit_at_in_current_command = false;
bool hit_lbrace_in_current_command = false;

bool waiting_to_terminate_while_statement = false;
bool can_increment_metatoken_counter = true;

enum lexer_special_mode_type {
	no_mode,
	mode_supershell,
	mode_subshell,
	mode_typecast,
	mode_arith,
	mode_reference,
	mode_array_assignment,
	mode_quote,
	mode_singlequote,
	mode_comment
};

SensibleStack<lexer_special_mode_type> modeStack;

inline bool contains_double_underscore(const std::string& s) {
	return s.find("__") != std::string::npos;
}

void emit(std::unique_ptr<antlr4::Token> t) {
	switch (t->getType()) {
		case WS:
			break;
		case DELIM:
		case NEWLINE:
		case BASH_WHILE_END:
		case CONNECTIVE:
		case DOUBLEAMPERSAND:
		case DOUBLEPIPE:
		case SUPERSHELL_START:
		case SUBSHELL_START:
		case DEPRECATED_SUBSHELL_START:
			incoming_token_can_be_lvalue = true;
			hit_at_in_current_command = false;
			hit_lbrace_in_current_command = false;
			break;
		case AT:
			if (hit_at_in_current_command) {
				incoming_token_can_be_lvalue = false;
			}
			hit_at_in_current_command = true;
			break;
		case LBRACE:
			if (hit_lbrace_in_current_command) {
				incoming_token_can_be_lvalue = false;
			}
			hit_lbrace_in_current_command = true;
			break;
		default:
			incoming_token_can_be_lvalue = false;
			break;
	}

	// Count meta-tokens
	if (t->getType() != WS && t->getText() != "\n" && modeStack.top() != mode_comment) {
		can_increment_metatoken_counter = true;
	} else if (modeStack.top() == no_mode && can_increment_metatoken_counter) {
		can_increment_metatoken_counter = false;
		metaTokenCount++;
	}


	antlr4::Lexer::emit(std::move(t));
}

#define emit(tokenType, text) emit(std::make_unique<CommonToken>(tokenType, text))

}

ESCAPE: '\\' . {
	// Don't escape if we're in a single-quoted string
	if (modeStack.top() == mode_singlequote) {
		if (getText() == "\\'") {
			emit(SINGLEQUOTE_END, "\\'");
			modeStack.pop();
		} else {
			emit(ESCAPE_LITERAL, getText());
		}
	} else if (getText() == "\\@") {
		emit(AT_LITERAL, "@");
	}
};

ESCAPE_LITERAL: '\\' .; // Dummy token

// Supershells
SUPERSHELL_START: '@(' {
	// Per Bash conventions, no expansion takes place within a single-quoted string
	// So don't start a supershell if we're in a single-quoted string
	switch (modeStack.top()) {
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

AT: '@' {
	if (_input->LA(1) == '{') {
		modeStack.push(mode_reference);
	}
};

AT_LITERAL: '@'; // Another dummy token

// Whitespace
WS: [ \t\r]+;

// Delimiters (newlines and semicolons, as in Bash)
NEWLINE: '\n' {
	switch (modeStack.top()) {
		case mode_comment:
			modeStack.pop();
			break;
		default:
			emit(DELIM, "\n");
			if (waiting_to_terminate_while_statement) {
				emit(BASH_WHILE_END, "\n");
				waiting_to_terminate_while_statement = false;
			}
			break;
	}
};

CONNECTIVE: DOUBLEAMPERSAND | DOUBLEPIPE; 
DOUBLEAMPERSAND: '&&';
DOUBLEPIPE: '||';

DELIM: '\n'; // Another dummy token

// Bash variables
BASH_VAR: '$' IDENTIFIER
		| '$' '{' IDENTIFIER '}'
		| '$#';

// Comments
COMMENT: '#' {
	switch (modeStack.top()) {
		case mode_reference:
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
	switch (modeStack.top()) {
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
	switch (modeStack.top()) {
		case mode_singlequote:
			emit(SINGLEQUOTE_END, "'");
			modeStack.pop();
			break;
		case mode_quote:
		case mode_comment:
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
KEYWORD_INCLUDE_ONCE: 'include_once';
KEYWORD_INCLUDE: 'include';
KEYWORD_THIS: 'this' {
	if (incoming_token_can_be_lvalue) {
		emit(KEYWORD_THIS_LVALUE, getText());
	}
};

KEYWORD_THIS_LVALUE: 'this'; // Another dummy token

KEYWORD_UPCAST: '@upcast' {
	modeStack.push(mode_typecast);
};
KEYWORD_DOWNCAST: '@downcast' {
	modeStack.push(mode_typecast);
};
KEYWORD_CAST: '@cast' {
	modeStack.push(mode_typecast);
};

// Bash keywords
BASH_KEYWORD_IF: 'if' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue) {
				IfDepth++;
			} else {
				emit(IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_ELIF: 'elif' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (!(incoming_token_can_be_lvalue && IfDepth > 0)) {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_THEN: 'then' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (!(incoming_token_can_be_lvalue && IfDepth > 0)) {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_ELSE: 'else' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (!(incoming_token_can_be_lvalue && IfDepth > 0)) {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_FI: 'fi' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue && IfDepth > 0) {
				IfDepth--;
			} else {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_WHILE: 'while' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue) {
				waiting_to_terminate_while_statement = true;
			} else {
				emit(IDENTIFIER_LVALUE, getText());
			}
			break;
	}
};

BASH_KEYWORD_CASE: 'case' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue) {
				CaseDepth++;
				most_recent_case_keyword_metatoken_position = metaTokenCount;
			} else {
				emit(IDENTIFIER, getText());
			}
	}
};

/*
How can we handle the *in* keyword in a case statement?
An ordinary case statement looks something like this:
	case $var in
		1)
			echo "One"
			;;
		2)
			echo "Two"
			;;
		*)
			echo "Not one or two"
			;;
	esac

Consider a slightly more obscure (but perfectly valid) case:
	case in in
		in)
			in=in
			./in in
			;;
	esac

Here we have several 'in's. Which one is the keyword?
	(Of course the second one is the keyword here)

One obvious thought is that: maybe it's sufficient to check if 'in' is the *last* non-whitespace token of the current line
Another potential approach is to check if 'in' is the second non-whitespace token after 'case'

But consider the following (still perfectly valid) Bash code:
	case "$(echo "a b c") d e f" #comment
	in "a b c d e f")
			echo "Match"
			;;
	esac

In this case, the 'in' keyword is not the last non-whitespace token of the current line,
nor is it the second non-whitespace token after 'case'

We can define a new term: 'meta-token'. By our definition, 'in' must be the second "meta-token" after 'case'
What is a meta-token?
	Whitespace does not count as a meta-token
	Comments do not count as meta-tokens
	Constructs such as quotes, subshells, supershells, and arithmetic expressions are *single* meta-tokens
		The parser for Bash++ actually considers strings to be sequences of statements
		(This is because there may be object references etc inside strings which we need to resolve)
		Therefore, a string is not a single token -- it is composed of many tokens.
	But here we define a "meta-token" in a way such that, for example:
		"$(echo "a b c") d e f"
	Is a single meta-token
		(And therefore, 'in' is the second meta-token after 'case' in the above example)
	We take the highest-level of these multi-token constructs (strings, subshells, etc)
		In the above case, the highest level is the outer string -- the quote marks surrounding the whole expression
	And we wait for this construct to terminate before we consider the next "meta-token"

Now that that's out of the way, we can say with complete accuracy,
	That in a properly-written 'case' statement,
	The 'in' keyword must be the second meta-token after 'case'.

 */

BASH_KEYWORD_IN: 'in' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (metaTokenCount != most_recent_case_keyword_metatoken_position + 2) {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
	}
};

BASH_KEYWORD_ESAC: 'esac' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue && CaseDepth > 0) {
				CaseDepth--;
			} else {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
	}
};

BASH_CASE_PATTERN_DELIM: ';;';

BASH_WHILE_END: 'while'; // Another in a long list of dummy tokens

SEMICOLON: ';' {
	emit(DELIM, ";");
};

// Identifiers
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]* {
	if (contains_double_underscore(getText())) {
		emit(INVALID_IDENTIFIER, getText());
	} else if (incoming_token_can_be_lvalue) {
		emit(IDENTIFIER_LVALUE, getText());
	}
};

INVALID_IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*; // Another dummy token

IDENTIFIER_LVALUE: [a-zA-Z_][a-zA-Z0-9_]*; // Yet another dummy token

// Operators
ASSIGN: '=' {
	switch (modeStack.top()) {
		case mode_reference:
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
			break;
		default:
			if (_input->LA(1) == '(') {
				modeStack.push(mode_array_assignment);
			}
			break;
	}
};

DOT: '.';

LBRACE: '{' {
	switch (modeStack.top()) {
		case mode_reference:
		case no_mode:
			if (braceDepth == 0) {
				emit(LBRACE_ROOTLEVEL, getText());
			}
			braceDepth++;
			break;
	}
};


RBRACE: '}' {
	switch (modeStack.top()) {
		case mode_reference:
			modeStack.pop();
			// Fall through
		case no_mode:
			braceDepth = std::max(braceDepth - 1, 0);
			if (braceDepth == 0) {
				emit(RBRACE_ROOTLEVEL, getText());
			}
			break;
	}
};

LBRACE_ROOTLEVEL: '{'; // Another dummy token
RBRACE_ROOTLEVEL: '}'; // Yet another dummy token

BACKTICK: '`' {
	switch (modeStack.top()) {
		case mode_singlequote:
		case mode_comment:
			// Just emit the BACKTICK token
			break;
		default:
			if (!inDeprecatedSubshell) {
				emit(DEPRECATED_SUBSHELL_START, "`");
				inDeprecatedSubshell = true;
			} else {
				emit(DEPRECATED_SUBSHELL_END, "`");
				inDeprecatedSubshell = false;
			}
			break;
	}
};

DEPRECATED_SUBSHELL_START: '`'; // Another dummy token
DEPRECATED_SUBSHELL_END: '`'; // Yet another dummy token

BASH_ARITH_START: '$((' {
	switch (modeStack.top()) {
		case mode_singlequote:
		case mode_comment:
			// Emit a dummy token
			emit(BASH_ARITH_LITERAL, "$((");
			break;
		default:
			modeStack.push(mode_arith);
			nestedArithStack.push(parenDepth);
			emit(BASH_ARITH_START, "$((");
			break;
	}
	parenDepth += 2;
};

BASH_ARITH_LITERAL: '$(('; // Another dummy token

SUBSHELL_START: '$(' {
	switch (modeStack.top()) {
		case mode_singlequote:
		case mode_comment:
			// Emit a dummy token
			emit(SUBSHELL_LITERAL, "$(");
			break;
		default:
			modeStack.push(mode_subshell);
			nestedSubshellStack.push(parenDepth);
			if (!inSubshell) {
				initialSubshellDepth = parenDepth;
			}
			inSubshell = true;
			emit(SUBSHELL_START, "$(");
			break;
	}
	parenDepth++;
};

SUBSHELL_LITERAL: '$('; // Another dummy token

DOLLAR: '$' {
	if (_input->LA(1) == '{') {
		modeStack.push(mode_reference);
	}
};

LPAREN: '(' {
	switch (modeStack.top()) {
		case mode_typecast:
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
			// Just emit the LPAREN token
			break;
		case mode_array_assignment:
			emit(ARRAY_ASSIGN_START, "(");
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

RPAREN: ')' {
	parenDepth--;
	switch (modeStack.top()) {
		case mode_array_assignment:
			emit(ARRAY_ASSIGN_END, ")");
			// Fall through
		case mode_typecast:
			modeStack.pop();
			break;
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
			// Just emit the RPAREN token
			break;
		case mode_arith:
			if ((parenDepth - 1) == nestedArithStack.top() && _input->LA(1) == ')') {
				// Translation:
				// If the current token is a right parenthesis and the next token is also a right parenthesis,
				// And these two match the top of the nestedArithStack, then we're at the end of the arithmetic expression
				nestedArithStack.pop();
				modeStack.pop();
				emit(BASH_ARITH_END, ")");
			}
			break;
		case mode_subshell:
			if (parenDepth == nestedSubshellStack.top()) {
				if (parenDepth == initialSubshellDepth) {
					inSubshell = false;
				}
				nestedSubshellStack.pop();
				modeStack.pop();
				emit(SUBSHELL_END, ")");
			}
			break;
		case mode_supershell:
			if (parenDepth == nestedSupershellStack.top()) {
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

BASH_ARITH_END: ')'; // This is a dummy token to make the lexer happy
					// The actual end of a bash arithmetic expression is detected by the RPAREN rule (above)

ARRAY_ASSIGN_START: '('; // Another dummy token
ARRAY_ASSIGN_END: ')'; // Yet another dummy token

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
