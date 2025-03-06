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
int ForDepth = 0;
int WhileDepth = 0;
int64_t most_recent_case_keyword_metatoken_position = INT64_MIN;
int64_t most_recent_for_keyword_metatoken_position = INT64_MIN;
SensibleStack<int> nestedSupershellStack;
SensibleStack<int> nestedSubshellStack;
SensibleStack<int> nestedArithStack;
SensibleStringStack nestedHeredocStack;
bool inSupershell = false;
bool inSubshell = false;
bool inDeprecatedSubshell = false;
bool waiting_for_heredoc_terminator = false;
bool waiting_for_heredoc_content_start = false;
bool parsing_method = false;

int64_t metaTokenCount = 0;

bool incoming_token_can_be_lvalue = true;
bool hit_at_in_current_command = false;
bool hit_lbrace_in_current_command = false;
bool hit_asterisk_in_current_command = false;

bool can_increment_metatoken_counter = true;

bool parsing_include_path = false;

bool last_token_was_lvalue = false;
bool in_variable_assignment_before_command = false;

enum lexer_special_mode_type {
	no_mode = 0,
	mode_supershell,
	mode_subshell,
	mode_arith,
	mode_reference,
	mode_primitive_reference,
	mode_array_assignment,
	mode_quote,
	mode_singlequote,
	mode_heredoc,
	mode_comment
};

SensibleStack<lexer_special_mode_type> modeStack;

enum for_or_while {
	neither = 0,
	for_loop,
	while_loop
};

SensibleStack<for_or_while> forWhileStack;

inline bool contains_double_underscore(const std::string& s) {
	return s.find("__") != std::string::npos;
}

void emit(std::unique_ptr<antlr4::Token> t) {
	last_token_was_lvalue = false;
	switch (t->getType()) {
		case WS:
			if (in_variable_assignment_before_command && modeStack.top() == no_mode) {
				in_variable_assignment_before_command = false;
				incoming_token_can_be_lvalue = true;
				hit_at_in_current_command = false;
				hit_lbrace_in_current_command = false;
				hit_asterisk_in_current_command = false;
			}
			break;
		case DELIM:
		case NEWLINE:
		case CONNECTIVE:
			if (modeStack.top() == no_mode) {
				in_variable_assignment_before_command = false;
			}
		case SUPERSHELL_START:
		case SUBSHELL_START:
		case DEPRECATED_SUBSHELL_START:
			incoming_token_can_be_lvalue = true;
			hit_at_in_current_command = false;
			hit_lbrace_in_current_command = false;
			hit_asterisk_in_current_command = false;
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
		case ASTERISK:
			if (hit_asterisk_in_current_command) {
				incoming_token_can_be_lvalue = false;
			}
			hit_asterisk_in_current_command = true;
			break;
		case IDENTIFIER_LVALUE:
		case KEYWORD_THIS_LVALUE:
			last_token_was_lvalue = true;
		default:
			incoming_token_can_be_lvalue = false;
			break;
	}

	// Count meta-tokens
	bool typeOk = false;
	switch (t->getType()) {
		case KEYWORD_CLASS:
		case KEYWORD_PUBLIC:
		case KEYWORD_PRIVATE:
		case KEYWORD_PROTECTED:
		case KEYWORD_VIRTUAL:
		case KEYWORD_METHOD:
		case KEYWORD_CONSTRUCTOR:
		case KEYWORD_DESTRUCTOR:
		case KEYWORD_NEW:
		case KEYWORD_DELETE:
		case KEYWORD_INCLUDE_ONCE:
		case KEYWORD_INCLUDE:
			can_increment_metatoken_counter = true;
		case WS:
			break;
		default:
			typeOk = true;
	}
	if (typeOk && t->getText() != "\n" && modeStack.top() != mode_comment) {
		can_increment_metatoken_counter = true;
	} else if (modeStack.top() == no_mode && can_increment_metatoken_counter) {
		can_increment_metatoken_counter = false;
		metaTokenCount++;
	}


	antlr4::Lexer::emit(std::move(t));
}

#define emit(tokenType, text) emit(std::make_unique<CommonToken>(tokenType, text))

}

/*
---------------------------------------------------------
---------------------------------------------------------
LVALUES AND RVALUES
---------------------------------------------------------
---------------------------------------------------------
In Bash++, we have a concept of "lvalues" and "rvalues".
	The way we use these two terms is not exactly the same as how they're used in some other languages

The most simple case is an assignment statement:
	@object.member=@otherObject.otherMember

If you're familiar with these terms (lvalue and rvalue), then you'll know which is which here
	@object.member is the lvalue in the assignment -- 'l' meaning 'left-hand side'
	@otherObject.otherMember is the rvalue in the assignment -- 'r' meaning 'right-hand side'
They serve different roles in the statement. And we have to handle those references differently
	The 'lvalue' should be resolved as a *place* to put some data in
	The 'rvalue' should be resolved to the data which is to be put in that place
	We should take care to interpret these two parts of the statement correctly

However, in Bash++ we have an additional meaning to the terms 'lvalue' and 'rvalue'
	Consider a simple shell command:
		program-name arg1 arg2 arg3
	Here, 'program-name' is the lvalue
		And 'arg1', 'arg2', and 'arg3' are the rvalues
	So, 'lvalue' can also mean 'the command to run', as in, 'the left-hand side of the command string'
	It's important for us to know which token is the command to run and which is the argument.

	Suppose a case in which we call a non-primitive object's method:
		1. @object.method arg1 arg2 arg3
		2. program-name @object.method arg2 arg3
	In the first case, '@object.method' is the lvalue and 'arg1', 'arg2', and 'arg3' are the rvalues
	In the second case, 'program-name' is the lvalue and '@object.method', 'arg2', and 'arg3' are the rvalues
		When the method is an rvalue:
			We should call the method and substitute its output in place of the method call
			Ie, pass the output of @object.method as an argument to program-name
		When the method is an lvalue:
			We just call it the same way we'd call any other command

OK. How do we determine if a given token is an lvalue?
	The naive approach is to say that the first token in a command is the lvalue
		Ie, whenever we hit a newline, a semicolon, a pipe, or a logical connective,
			We know that what's coming next can be an lvalue
	But consider the following:
		environmentVariable=value program-name arg1 arg2 arg3
	Here, 'program-name' is *very clearly* an lvalue -- even though it's not the first token in the command
	In fact, we can have an arbitrarily long list of shell variable assignments before the command
		var1=val1 var2=val2 var3=val3 program-name arg1 arg2 arg3
	Meanwhile, the left-hand sides of each assignment before 'command' are also lvalues
		And it's important for us to *know* that they're lvalues

Fortunately, these shell variable assignments prefacing a command are the only exceptional cases
*/



/*
---------------------------------------------------------
---------------------------------------------------------
META-TOKENS
---------------------------------------------------------
---------------------------------------------------------
How can we handle the *in* keyword in a 'case' (or 'for') statement?
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
			if (waiting_for_heredoc_content_start) {
				emit(HEREDOC_CONTENT_START, "\n");
				waiting_for_heredoc_content_start = false;
				modeStack.push(mode_heredoc);
			}
			break;
	}
};

DOUBLEAMPERSAND: '&&' {
	switch (modeStack.top()) {
		case mode_comment:
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
			emit(CATCHALL, "&&");
			break;
		default:
			emit(CONNECTIVE, "&&");
			break;
	}
};
DOUBLEPIPE: '||' {
	switch (modeStack.top()) {
		case mode_comment:
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
			emit(CATCHALL, "||");
			break;
		default:
			emit(CONNECTIVE, "||");
			break;
	}
};
PIPE: '|' {
	switch (modeStack.top()) {
		case mode_comment:
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
			emit(CATCHALL, "|");
			break;
		default:
			emit(CONNECTIVE, "|");
			break;
	}
};

CONNECTIVE: '&&'; // Another dummy token

DELIM: '\n'; // Another dummy token

// Bash variables
BASH_VAR: '$' IDENTIFIER
		| '$' '{' IDENTIFIER '}'
		| '$#';

// Comments
COMMENT: '#' {
	switch (modeStack.top()) {
		case mode_reference:
		case mode_primitive_reference:
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
		case mode_comment:
			emit(POUNDKEY, "#");
			break;
		default:
			modeStack.push(mode_comment);
			break;
	}
};

POUNDKEY: '#'; // Yet another dummy token

// Heredocs
HERESTRING_START: '<<<';

HEREDOC_START: '<<' {
	switch (modeStack.top()) {
		case mode_arith:
		case mode_reference:
		case mode_primitive_reference:
		case mode_array_assignment:
		case mode_quote:
		case mode_singlequote:
		case mode_comment:
		case mode_heredoc:
			emit(HEREDOC_LITERAL, "<<");
			break;
		default:
			waiting_for_heredoc_terminator = true;
			break;
	}
};

HEREDOC_LITERAL: '<<'; // Another dummy token

HEREDOC_CONTENT_START: '<<'; // Yet another dummy token

// Strings
QUOTE: '"' {
	switch (modeStack.top()) {
		case mode_quote:
			emit(QUOTE_END, "\"");
			modeStack.pop();
			break;
		case mode_comment:
		case mode_heredoc:
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
		case mode_heredoc:
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
KEYWORD_CLASS: '@class' WORD_BOUNDARY;
KEYWORD_PUBLIC: '@public' WORD_BOUNDARY;
KEYWORD_PRIVATE: '@private' WORD_BOUNDARY;
KEYWORD_PROTECTED: '@protected' WORD_BOUNDARY;
KEYWORD_VIRTUAL: '@virtual' WORD_BOUNDARY;

KEYWORD_METHOD: '@method' WORD_BOUNDARY {
	if (modeStack.top() == no_mode) {
		parsing_method = true;
	}
};

KEYWORD_CONSTRUCTOR: '@constructor' WORD_BOUNDARY;
KEYWORD_DESTRUCTOR: '@destructor' WORD_BOUNDARY;
KEYWORD_NEW: '@new' WORD_BOUNDARY;
KEYWORD_DELETE: '@delete' WORD_BOUNDARY;
KEYWORD_NULLPTR: '@nullptr';

KEYWORD_DYNAMIC_CAST: '@dynamic_cast';

KEYWORD_INCLUDE_ONCE: '@include_once' WORD_BOUNDARY {
	if (modeStack.top() == no_mode) {
		pushMode(PARSE_INCLUDE_PATH);
	}
};
KEYWORD_INCLUDE: '@include' WORD_BOUNDARY {
	if (modeStack.top() == no_mode) {
		pushMode(PARSE_INCLUDE_PATH);
	}
};

KEYWORD_THIS: 'this' {
	if (incoming_token_can_be_lvalue) {
		emit(KEYWORD_THIS_LVALUE, getText());
	}
};

KEYWORD_THIS_LVALUE: 'this'; // Another dummy token

// Bash keywords
BASH_KEYWORD_IF: 'if' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
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
		case mode_heredoc:
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
		case mode_heredoc:
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
		case mode_heredoc:
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
		case mode_heredoc:
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
		case mode_heredoc:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue) {
				WhileDepth++;
				forWhileStack.push(for_or_while::while_loop);
			} else {
				emit(IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_DO: 'do' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if ( !( incoming_token_can_be_lvalue && (WhileDepth > 0 || ForDepth > 0) ) ) {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_DONE: 'done' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue) {
				if (forWhileStack.top() == for_or_while::while_loop && WhileDepth > 0) {
					WhileDepth--;
				} else if (forWhileStack.top() == for_or_while::for_loop && ForDepth > 0) {
					ForDepth--;
				}
				forWhileStack.pop();
			} else {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
			break;
	}
};

BASH_KEYWORD_CASE: 'case' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
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

BASH_KEYWORD_FOR: 'for' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (incoming_token_can_be_lvalue) {
				ForDepth++;
				most_recent_for_keyword_metatoken_position = metaTokenCount;
				forWhileStack.push(for_or_while::for_loop);
			} else {
				emit(IDENTIFIER, getText());
			}
	}
};

BASH_KEYWORD_IN: 'in' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
		case mode_comment:
		case mode_arith:
		case mode_array_assignment:
			emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			break;
		default:
			if (metaTokenCount != most_recent_case_keyword_metatoken_position + 2
				&& metaTokenCount != most_recent_for_keyword_metatoken_position + 2) {
				emit(incoming_token_can_be_lvalue ? IDENTIFIER_LVALUE : IDENTIFIER, getText());
			}
	}
};

BASH_KEYWORD_ESAC: 'esac' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
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

	if (waiting_for_heredoc_terminator) {
		waiting_for_heredoc_terminator = false;
		waiting_for_heredoc_content_start = true;
		nestedHeredocStack.push(getText());
	}

	if (modeStack.top() == mode_heredoc && getText() == nestedHeredocStack.top()) {
		modeStack.pop();
		nestedHeredocStack.pop();
		emit(HEREDOC_END, getText());
	}
};

HEREDOC_END: [a-zA-Z_][a-zA-Z0-9_]*; // Another dummy token

INVALID_IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*; // Another dummy token

IDENTIFIER_LVALUE: [a-zA-Z_][a-zA-Z0-9_]*; // Yet another dummy token

// Operators
ASSIGN: '=' {
	switch (modeStack.top()) {
		case mode_reference:
		case mode_primitive_reference:
		case mode_quote:
		case mode_heredoc:
		case mode_singlequote:
		case mode_comment:
			break;
		default:
			if (_input->LA(1) == '(') {
				modeStack.push(mode_array_assignment);
			}
			in_variable_assignment_before_command = last_token_was_lvalue;
			break;
	}
};

DOT: '.';

LBRACE: '{' {
	switch (modeStack.top()) {
		case mode_reference:
		case mode_primitive_reference:
		case no_mode:
			if (braceDepth == 0) {
				emit(LBRACE_ROOTLEVEL, getText());
			} else if (parsing_method && braceDepth == 1) {
				emit(METHOD_START, getText());
			}
			braceDepth++;
			break;
	}
};


RBRACE: '}' {
	switch (modeStack.top()) {
		case mode_reference:
		case mode_primitive_reference:
			modeStack.pop();
			// Fall through
		case no_mode:
			braceDepth = std::max(braceDepth - 1, 0);
			if (braceDepth == 0) {
				emit(RBRACE_ROOTLEVEL, getText());
			} else if (parsing_method && braceDepth == 1) {
				emit(METHOD_END, getText());
				parsing_method = false;
			}
			break;
	}
};

LBRACE_ROOTLEVEL: '{'; // Another dummy token
RBRACE_ROOTLEVEL: '}'; // Yet another dummy token
METHOD_START: '{'; // Yet another dummy token
METHOD_END: '}'; // Yet another dummy token

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
		modeStack.push(mode_primitive_reference);
	}
};

LPAREN: '(' {
	switch (modeStack.top()) {
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
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
			modeStack.pop();
			break;
		case mode_quote:
		case mode_singlequote:
		case mode_heredoc:
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

LBRACKET: '[' {
	if (modeStack.top() == mode_reference) {
		emit(ARRAY_INDEX_START, "[");
	}
};
RBRACKET: ']' {
	if (modeStack.top() == mode_reference) {
		emit(ARRAY_INDEX_END, "]");
	}
};

ARRAY_INDEX_START: '['; // Another dummy token
ARRAY_INDEX_END: ']'; // Yet another dummy token

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

WORD_BOUNDARY: EOF | ~[A-Za-z0-9_];

mode PARSE_INCLUDE_PATH;

INCLUDE_PATH_WS: [ \t\r]+ -> skip;

INCLUDE_PATH_START: '<';
INCLUDE_PATH_END: '>' -> popMode;

LOCAL_INCLUDE_PATH_START: '"' {
	if (parsing_include_path) {
		parsing_include_path = false;
		emit(INCLUDE_PATH_END, "\"");
		popMode();
	} else {
		parsing_include_path = true;
	}
};

INCLUDE_PATH: ~["<>\n]+;
