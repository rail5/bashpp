

%{
#include "parser.tab.hpp" // Bison header for token types
#include "../ModeStack.h"
#include <cstdint>
#include <stack>

/**
GNU Bison Docs, 10.1.7.2: Complete Symbols

> With both %define api.value.type variant and
> %define api.token.constructor, the parser defines
> the type symbol_type, and expects yylex to have
> the following prototype.
>
> Function: parser::symbol_type yylex ()
>
> Function: parser::symbol_type yylex (type1 arg1, ...)
>
> Return a complete symbol, aggregating its type
> (i.e.,the traditional value returned by yylex),
> its semantic value, and possibly its location.
> Invocations of ‘%lex-param {type1 arg1}’ yield
> additional arguments.

*/
extern yyscan_t main_lexer;

#define YY_DECL yy::parser::symbol_type yylex(yyscan_t yyscanner)
YY_DECL;

/* For compatibility with Bison's expectations, as documented above */
yy::parser::symbol_type yylex() {
	return ::yylex(main_lexer);
}

extern ModeStack modeStack; // Stack to manage lexer modes

/**
 * @struct LexerState
 * @brief A struct to contain information that we need to keep track of during lexing,
 * such as whether an incoming token could be an lvalue,
 * or the current nesting depth of backtick-subshells.
 */
struct LexerState {
	std::string singlequote_contents;
	uint32_t token_count_inside_bracereference = 0;
	uint32_t deprecated_subshell_nesting_depth = 0;
	std::stack<uint32_t> deprecated_subshell_nesting_depths;
	std::string heredoc_delimiter;
	bool bash_case_input_received = false;
	bool bash_case_keyword_in_received = false;
	bool bash_for_or_select_variable_received = false;
	bool bash_for_or_select_input_received = false;
	bool parsed_assignment_operator = false;
	bool incoming_token_can_be_lvalue = true;

	void reset() {
		singlequote_contents.clear();
		token_count_inside_bracereference = 0;
		deprecated_subshell_nesting_depth = 0;
		while (!deprecated_subshell_nesting_depths.empty()) {
			deprecated_subshell_nesting_depths.pop();
		}
		heredoc_delimiter.clear();
		bash_case_input_received = false;
		bash_case_keyword_in_received = false;
		bash_for_or_select_variable_received = false;
		bash_for_or_select_input_received = false;
		parsed_assignment_operator = false;
		incoming_token_can_be_lvalue = true;
	}
};

static LexerState lexerState;

/* For external use by the parser
 * After the parser parses an assignment statement,
 * It can use this function to signal back to the lexer
 * That the next token could be an lvalue
 */
void set_incoming_token_can_be_lvalue(bool canBeLvalue) {
	lexerState.incoming_token_can_be_lvalue = canBeLvalue;
}

void set_bash_case_input_received(bool received) {
	lexerState.bash_case_input_received = received;
}

void set_bash_for_or_select_variable_received(bool received) {
	lexerState.bash_for_or_select_variable_received = received;
}

void set_parsed_assignment_operator(bool parsed) {
	lexerState.parsed_assignment_operator = parsed;
}

/**
 * @brief Determines if the current token can be treated as an lvalue, and updates lexer state accordingly.
 *
 * @param token The current token being processed.
 * @return Either the original token unmodified, or an lvalue variant of the token if applicable.
*/
yy::parser::symbol_type maybe_get_lvalue_token(yy::parser::symbol_type token) {
	yy::parser::symbol_kind_type type = token.kind();
	switch (type) {
		case yy::parser::symbol_kind::S_AT:
			// If this AT is the first non-whitespace token of the new command string, it's an lvalue
			if (lexerState.incoming_token_can_be_lvalue) {
				lexerState.incoming_token_can_be_lvalue = false; // Next token cannot be an lvalue
				return yy::parser::make_AT_LVALUE();
			}
			break;
		case yy::parser::symbol_kind::S_REF_START:
			// Same as above
			if (lexerState.incoming_token_can_be_lvalue) {
				lexerState.incoming_token_can_be_lvalue = false;
				return yy::parser::make_REF_START_LVALUE();
			}
			break;
		case yy::parser::symbol_kind::S_IDENTIFIER:
			// Same as above
			if (lexerState.incoming_token_can_be_lvalue) {
				lexerState.incoming_token_can_be_lvalue = false;
				std::string text = token.value.as<std::string>();
				return yy::parser::make_IDENTIFIER_LVALUE(text);
			}
			break;
		case yy::parser::symbol_kind::S_KEYWORD_THIS:
			// Same as above
			if (lexerState.incoming_token_can_be_lvalue) {
				lexerState.incoming_token_can_be_lvalue = false;
				return yy::parser::make_KEYWORD_THIS_LVALUE();
			}
			break;
		case yy::parser::symbol_kind::S_KEYWORD_SUPER:
			// Same as above
			if (lexerState.incoming_token_can_be_lvalue) {
				lexerState.incoming_token_can_be_lvalue = false;
				return yy::parser::make_KEYWORD_SUPER_LVALUE();
			}
			break;
		case yy::parser::symbol_kind::S_DELIM:
		case yy::parser::symbol_kind::S_DOUBLEAMPERSAND:
		case yy::parser::symbol_kind::S_DOUBLEPIPE:
		case yy::parser::symbol_kind::S_PIPE:
		case yy::parser::symbol_kind::S_SUPERSHELL_START:
		case yy::parser::symbol_kind::S_SUBSHELL_START:
		case yy::parser::symbol_kind::S_SUBSHELL_SUBSTITUTION_START:
		case yy::parser::symbol_kind::S_DEPRECATED_SUBSHELL_START:
		case yy::parser::symbol_kind::S_ARITH_FOR_CONDITION_START:
			// At the start of a new command string, the next token can be an lvalue
			lexerState.incoming_token_can_be_lvalue = true;
			break;
		case yy::parser::symbol_kind::S_KEYWORD_PUBLIC:
		case yy::parser::symbol_kind::S_KEYWORD_PROTECTED:
		case yy::parser::symbol_kind::S_KEYWORD_PRIVATE:
			// Access specifiers do not change lvalue considerations
			break;
		case yy::parser::symbol_kind::S_WS:
			// Whitespace does not change lvalue considerations
			break;
		case yy::parser::symbol_kind::S_DEREFERENCE_OPERATOR:
			// Dereference operator does not change lvalue considerations
			break;
		default:
			lexerState.incoming_token_can_be_lvalue = false;
			break;
	}
	return token; // If we're here, just return the original token unmodified
}

void updateLexerState() {
	lexerState.parsed_assignment_operator = false;
}

%}

%option noyywrap
%option stack
%option reentrant

ESCAPED_CHAR    \\(.|\n)
AT              @

WS              [ \t]+

DELIM           [\n;]

IDENTIFIER              [a-zA-Z_][a-zA-Z0-9_]*
INTEGER                 [0-9]+

/* Bash++ keywords */
KEYWORD_NEW             @new
KEYWORD_DELETE          @delete
KEYWORD_DYNAMIC_CAST    @dynamic_cast
KEYWORD_INCLUDE_ONCE    @include_once
KEYWORD_INCLUDE         @include
KEYWORD_NULLPTR	        @nullptr
KEYWORD_CLASS           @class
KEYWORD_METHOD          @method
KEYWORD_CONSTRUCTOR     @constructor
KEYWORD_DESTRUCTOR      @destructor
KEYWORD_PUBLIC          @public
KEYWORD_PROTECTED       @protected
KEYWORD_PRIVATE         @private
KEYWORD_THIS            @this
KEYWORD_SUPER           @super
KEYWORD_TYPEOF          @typeof
KEYWORD_VIRTUAL         @virtual

LANGLE                  [<]
RANGLE                  [>]

LBRACE                  [{]
RBRACE                  [}]

DOUBLEAMPERSAND         "&&"
DOUBLEPIPE              "||"
PIPE                    "|"

/* Tokens only signifcant to @include statements */
SPECIAL_KEYWORD_DYNAMIC   dynamic
SPECIAL_KEYWORD_STATIC    static
SPECIAL_KEYWORD_AS        as
QUOTED_INCLUDE_PATH       \"(\\.|[^\"])+\"
ANGLEBRACKET_INCLUDE_PATH <([^>])+>

/* Start conditions (lexer modes) */

/* Triggered when we hit an unescaped '@' */
%x BPP_MODE

%x BPP_REFERENCE_MODE

%x BPP_BRACEREFERENCE_MODE

/* Parsing @include statements */
%x BPP_INCLUDE_MODE

/* Handling single-quoted strings as single tokens */
%x SINGLEQUOTE_MODE

/* Handling double-quoted strings as sequences of STRING_TOKEN and interpolations */
%x DOUBLEQUOTE_MODE

/* Handling Bash shell variables */
%x BASH_VAR_MODE

%x HEREDOC_CONTENT_MODE

%x BASH_CASE_BODY_MODE

%x SKIP_AFTER_DELIM_MODE

/* INCLUSIVE start conditions
 * These start conditions are essentially used to lex normally while distinguishing special modes,
 * e.g., when parsing array indices (as in ${var[array_index]}),
 * Most lexing inside the  '[ ... ]' pair has to be done normally (normal lexing mode),
 * Except for the handling of the closing bracket ']'
 * In this case, it's nice to have some flag to check whether we're inside an array index or not,
 * Without disturbing the normal lexing rules.
 * Therefore we declare an inclusive start condition with no special rules of its own,
 * And inside the rule for ']', we check the top of the mode stack to see if we're in ARRAY_INDEX_MODE.
 */

%s ARRAY_INDEX_MODE

%s SUPERSHELL_MODE

%s SUBSHELL_MODE

%s SUBSHELL_SUBSTITUTION_MODE

%s DEPRECATED_SUBSHELL_MODE

%s HEREDOC_MODE

%s BASH_CASE_MODE

%s BASH_CASE_PATTERN_ACTION_MODE

%s BASH_FOR_OR_SELECT_MODE

%s BASH_FOR_OR_SELECT_INPUT_MODE

%s BASH_FOR_OR_SELECT_BODY_MODE

%%

%{
	/**
	* Macro to emit a token with optional semantic value.
	* Usage:
	*   emit(TOKEN_TYPE); // No semantic value
	*   emit(TOKEN_TYPE, value); // With semantic value
	*
	* This macro constructs and returns a complete symbol
	* of the specified token type, optionally including
	* a semantic value.
	*
	* For example, emit(IDENTIFIER, "value") expands to:
	*  return yy::parser::make_IDENTIFIER("value");
	*
	* Or, emit(AT) expands to:
	*  return yy::parser::make_AT();
	*/
	#define emit(tokenType, ...) \
		updateLexerState(); \
		return maybe_get_lvalue_token(yy::parser::make_##tokenType(__VA_ARGS__));
	
	/**
	* Helper to get the text of the current token as a std::string.
	*/
	#define tokenText std::string(yytext, yyleng)
%}

<*>{
	[\r]+ { /* Ignore carriage returns globally */ }
}

<SKIP_AFTER_DELIM_MODE>{
	[ \t\n]+    { /* Ignore whitespace and newlines after a DELIM */ }
	[\#][^\n]*  { /* Ignore comments */ }
	.           {
		// Any other character means we're done skipping
		modeStack.pop(); // Exit SKIP_AFTER_DELIM_MODE
		yyless(0); // Reconsume the character
	}
}

{ESCAPED_CHAR}                       {
	std::string sequence = tokenText;
	if (modeStack.top() == DEPRECATED_SUBSHELL_MODE && sequence[1] == '`') {
		// Is this starting a nested subshell or is it finishing a nested subshell?
		lexerState.deprecated_subshell_nesting_depth++; // Include the latest backslash
		if (
			!lexerState.deprecated_subshell_nesting_depths.empty()
			&& lexerState.deprecated_subshell_nesting_depth == lexerState.deprecated_subshell_nesting_depths.top()
		) {
			// Finishing a nested subshell
			modeStack.pop();
			uint32_t depth = lexerState.deprecated_subshell_nesting_depth;
			lexerState.deprecated_subshell_nesting_depths.pop();
			lexerState.deprecated_subshell_nesting_depth = 0;
			emit(DEPRECATED_SUBSHELL_END, depth);
		} else {
			// Starting a new nested subshell
			modeStack.push(DEPRECATED_SUBSHELL_MODE);
			lexerState.deprecated_subshell_nesting_depths.push(lexerState.deprecated_subshell_nesting_depth);
			lexerState.deprecated_subshell_nesting_depth = 0;
			emit(DEPRECATED_SUBSHELL_START, lexerState.deprecated_subshell_nesting_depths.top());
		}
	} else if (modeStack.top() == DEPRECATED_SUBSHELL_MODE && sequence[1] == '\\') {
		lexerState.deprecated_subshell_nesting_depth++;
		// No need to emit anything yet
	} else {
		emit(ESCAPED_CHAR, tokenText);
	}
}
{WS}                                 {
	if (lexerState.parsed_assignment_operator) {
		// If whitespace immediately follows an assignment operator,
		// The rvalue of the assignment is empty
		yyless(0); // Reconsume the whitespace
		emit(EMPTY_ASSIGNMENT);
	}
	emit(WS, tokenText);
}

{AT}                                 { modeStack.push(BPP_MODE); yyless(0); }

<BPP_MODE>{
	{KEYWORD_INCLUDE}/[^a-zA-Z0-9_]      { modeStack.pop(); modeStack.push(BPP_INCLUDE_MODE); emit(KEYWORD_INCLUDE); }
	{KEYWORD_INCLUDE_ONCE}/[^a-zA-Z0-9_] { modeStack.pop(); modeStack.push(BPP_INCLUDE_MODE); emit(KEYWORD_INCLUDE_ONCE); }

	{KEYWORD_CLASS}/[^a-zA-Z0-9_]        { modeStack.pop(); emit(KEYWORD_CLASS); }
	{KEYWORD_PUBLIC}/[^a-zA-Z0-9_]       { modeStack.pop(); emit(KEYWORD_PUBLIC); }
	{KEYWORD_PROTECTED}/[^a-zA-Z0-9_]    { modeStack.pop(); emit(KEYWORD_PROTECTED); }
	{KEYWORD_PRIVATE}/[^a-zA-Z0-9_]      { modeStack.pop(); emit(KEYWORD_PRIVATE); }
	{KEYWORD_NEW}/[^a-zA-Z0-9_]          { modeStack.pop(); emit(KEYWORD_NEW); }
	{KEYWORD_DELETE}/[^a-zA-Z0-9_]       { modeStack.pop(); emit(KEYWORD_DELETE); }
	{KEYWORD_NULLPTR}/[^a-zA-Z0-9_]      { modeStack.pop(); emit(KEYWORD_NULLPTR); }
	{KEYWORD_METHOD}/[^a-zA-Z0-9_]       { modeStack.pop(); emit(KEYWORD_METHOD); }
	{KEYWORD_VIRTUAL}/[^a-zA-Z0-9_]      { modeStack.pop(); emit(KEYWORD_VIRTUAL); }
	{KEYWORD_CONSTRUCTOR}/[^a-zA-Z0-9_]  { modeStack.pop(); emit(KEYWORD_CONSTRUCTOR); }
	{KEYWORD_DESTRUCTOR}/[^a-zA-Z0-9_]   { modeStack.pop(); emit(KEYWORD_DESTRUCTOR); }
	{KEYWORD_DYNAMIC_CAST}/[^a-zA-Z0-9_] { modeStack.pop(); emit(KEYWORD_DYNAMIC_CAST); }
	{KEYWORD_TYPEOF}/[^a-zA-Z0-9_]       { modeStack.pop(); emit(KEYWORD_TYPEOF); }
	{KEYWORD_THIS}/[^a-zA-Z0-9_]         { modeStack.pop(); modeStack.push(BPP_REFERENCE_MODE); emit(KEYWORD_THIS); }
	{KEYWORD_SUPER}/[^a-zA-Z0-9_]        { modeStack.pop(); modeStack.push(BPP_REFERENCE_MODE); emit(KEYWORD_SUPER); }

	"@{"                                 { modeStack.pop(); modeStack.push(BPP_BRACEREFERENCE_MODE); emit(REF_START); }
	"@("                                 { modeStack.pop(); modeStack.push(SUPERSHELL_MODE); emit(SUPERSHELL_START); }
	{AT}                                 { modeStack.pop(); modeStack.push(BPP_REFERENCE_MODE); emit(AT); }
}

{DELIM}                              {
	switch (modeStack.top()) {
		case HEREDOC_MODE:
			if (tokenText == "\n") {
				// Do not push SKIP_AFTER_DELIM_MODE if we're going into a string-type mode like HEREDOC_CONTENT_MODE
				modeStack.push(HEREDOC_CONTENT_MODE);
			}
			break;
		case BASH_CASE_MODE:
			if (lexerState.bash_case_keyword_in_received) {
				// Beginning of the case body
				modeStack.pop(); // Exit BASH_CASE_MODE
				modeStack.push(BASH_CASE_BODY_MODE);
				lexerState.bash_case_keyword_in_received = false; // Reset for next time
				lexerState.bash_case_input_received = false; // Reset for next time
				modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
				emit(BASH_CASE_BODY_BEGIN, tokenText); // NOTE: CHANGE THIS IF emit() MACRO CHANGES
				// We are VERY MUCH relying on emit() returning immediately
			} else {
				modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
				emit(WS, tokenText);
			}
			break;
		case BASH_FOR_OR_SELECT_INPUT_MODE:
			modeStack.pop(); // Exit BASH_FOR_OR_SELECT_INPUT_MODE
			lexerState.bash_for_or_select_input_received = true; // So that we know 'do' can be a keyword
			lexerState.bash_for_or_select_variable_received = false; // Reset for next time
			modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
			emit(DELIM, tokenText); // NOTE: CHANGE THIS IF emit() MACRO CHANGES
			// Same deal as above
			break;
		case BASH_FOR_OR_SELECT_MODE:
			if (lexerState.bash_for_or_select_variable_received) {
				// No 'in' keyword; proceed to body directly
				modeStack.pop(); // Exit BASH_FOR_OR_SELECT_MODE
				lexerState.bash_for_or_select_input_received = true; // So that we know 'do' can be a keyword
				lexerState.bash_for_or_select_variable_received = false; // Reset for next time
				modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
				emit(DELIM, tokenText); // NOTE: CHANGE THIS IF emit() MACRO CHANGES
			}
			break;
		default:
			break;
	}

	if (lexerState.parsed_assignment_operator) {
		// If a DELIM immediately follows an assignment operator,
		// The rvalue of the assignment is empty
		yyless(0); // Reconsume the DELIM
		// Do not push SKIP_AFTER_DELIM_MODE yet, as we need to reconsume the DELIM first
		emit(EMPTY_ASSIGNMENT);
	}
	modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
	emit(DELIM, tokenText);
}
{DOUBLEAMPERSAND}                    { emit(DOUBLEAMPERSAND); }
{DOUBLEPIPE}                         { emit(DOUBLEPIPE); }
{PIPE}                               { emit(PIPE); }
{LBRACE}                             { emit(LBRACE); }
{RBRACE}                             { emit(RBRACE); }
":"                                  { emit(COLON); }
"+="                                 { emit(PLUS_EQUALS); }
"="                                  { emit(EQUALS); }
[\*]/[@]                             { emit(DEREFERENCE_OPERATOR); }
[*]                                  { emit(ASTERISK); }
"&"                                  { emit(AMPERSAND); }
{IDENTIFIER}                         {
	if (modeStack.top() == HEREDOC_MODE) {
		// This IDENTIFIER is the heredoc delimiter
		lexerState.heredoc_delimiter = tokenText;
		emit(HEREDOC_DELIMITER, tokenText);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}
{LANGLE}                             { emit(LANGLE); }
{RANGLE}                             { emit(RANGLE); }

"${"                                 { modeStack.push(BASH_VAR_MODE); emit(BASH_VAR_START); }

\${IDENTIFIER}                       { emit(BASH_VAR, tokenText); }
\$[\@\#\*\-\$\!\?]                   { emit(BASH_VAR, tokenText); }
\$\{[\@\#\*\-\$!\?]\}                { emit(BASH_VAR, tokenText); }
\${INTEGER}                          { emit(BASH_VAR, tokenText); }
\$\{{INTEGER}\}                      { emit(BASH_VAR, tokenText); }

"["                                  { emit(LBRACKET); }

"]"                                  {
	switch (modeStack.top()) {
		case ARRAY_INDEX_MODE:
			modeStack.pop(); // Exit ARRAY_INDEX_MODE
			emit(ARRAY_INDEX_END);
			break;
		default:
			emit(RBRACKET);
			break;
	}
}

"$(" {
	modeStack.push(SUBSHELL_SUBSTITUTION_MODE);
	emit(SUBSHELL_SUBSTITUTION_START);
}

"(" {
	modeStack.push(SUBSHELL_MODE);
	emit(SUBSHELL_START);
}

")" {
	switch (modeStack.top()) {
		case SUPERSHELL_MODE:
			modeStack.pop(); // Exit SUPERSHELL_MODE
			emit(SUPERSHELL_END);
			break;
		case SUBSHELL_MODE:
			modeStack.pop(); // Exit SUBSHELL_MODE
			emit(SUBSHELL_END);
			break;
		case SUBSHELL_SUBSTITUTION_MODE:
			modeStack.pop(); // Exit SUBSHELL_SUBSTITUTION_MODE
			emit(SUBSHELL_SUBSTITUTION_END);
			break;
		default:
			emit(RPAREN);
			break;
	}
}

"`" {
	switch (modeStack.top()) {
		case DEPRECATED_SUBSHELL_MODE:
			modeStack.pop(); // Exit DEPRECATED_SUBSHELL_MODE
			emit(DEPRECATED_SUBSHELL_END, 0);
			break;
		default:
			modeStack.push(DEPRECATED_SUBSHELL_MODE);
			lexerState.deprecated_subshell_nesting_depths.push(0);
			lexerState.deprecated_subshell_nesting_depth = 0;
			emit(DEPRECATED_SUBSHELL_START, 0);
			break;
	}
}

['] {
	modeStack.push(SINGLEQUOTE_MODE);
	lexerState.singlequote_contents.clear();
	lexerState.singlequote_contents += '\'';
}

["] {
	modeStack.push(DOUBLEQUOTE_MODE);
	emit(QUOTE_BEGIN);
}

"<<-" {
	modeStack.push(HEREDOC_MODE);
	emit(HEREDOC_START);
}

"<<<" { emit(HERESTRING_START); }

"<<" {
	modeStack.push(HEREDOC_MODE);
	emit(HEREDOC_START);
}

"case"/[ \t\n] {
	if (lexerState.incoming_token_can_be_lvalue) {
		lexerState.incoming_token_can_be_lvalue = false;
		modeStack.push(BASH_CASE_MODE);
		emit(BASH_KEYWORD_CASE);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"select"/[ \t\n] {
	if (lexerState.incoming_token_can_be_lvalue) {
		lexerState.incoming_token_can_be_lvalue = false;
		modeStack.push(BASH_FOR_OR_SELECT_MODE);
		emit(BASH_KEYWORD_SELECT);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"for"/[ \t\n] {
	if (lexerState.incoming_token_can_be_lvalue) {
		lexerState.incoming_token_can_be_lvalue = false;
		modeStack.push(BASH_FOR_OR_SELECT_MODE);
		emit(BASH_KEYWORD_FOR);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"do"/[ \t\n] {
	if (lexerState.bash_for_or_select_input_received) {
		lexerState.bash_for_or_select_input_received = false; // Reset for next time
		modeStack.push(BASH_FOR_OR_SELECT_BODY_MODE);
		emit(BASH_KEYWORD_DO);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

[\#][^\n]*  { /* Ignore comments */ }


<SINGLEQUOTE_MODE>{
	[^'\\]+ {
		/* Consume all characters until closing singlequote */
		lexerState.singlequote_contents += tokenText;
	}

	{ESCAPED_CHAR} {
		/* If it's an escaped singlequote, it's not the end of the string */
		lexerState.singlequote_contents += tokenText;
	}

	"'" {
		/* Closing singlequote found */
		modeStack.pop(); // Exit SINGLEQUOTE_MODE
		lexerState.singlequote_contents += '\'';
		emit(SINGLEQUOTED_STRING, lexerState.singlequote_contents);
	}
}

<DOUBLEQUOTE_MODE,HEREDOC_CONTENT_MODE,BASH_CASE_BODY_MODE>{
	/* The following rules are for handling string interpolations */
	\\@ {
		// Literal '@'
		emit(STRING_CONTENT, "@");
	}

	\\\$ {
		// Literal dollar sign
		emit(STRING_CONTENT, "$");
	}

	\\` {
		// Literal backtick
		emit(STRING_CONTENT, "`");
	}

	\\\\ {
		// Literal backslash
		emit(STRING_CONTENT, "\\");
	}

	\\[\*&] {
		// Literal '*' or '&'
		emit(STRING_CONTENT, tokenText.substr(1, 1));
	}

	"@(" {
		modeStack.push(SUPERSHELL_MODE);
		emit(SUPERSHELL_START);
	}

	"@{" {
		modeStack.push(BPP_BRACEREFERENCE_MODE);
		emit(REF_START);
	}

	"$(" {
		modeStack.push(SUBSHELL_SUBSTITUTION_MODE);
		emit(SUBSHELL_SUBSTITUTION_START);
	}

	"$" {
		// Solitary $
		emit(STRING_CONTENT, "$");
	}

	"`" {
		modeStack.push(DEPRECATED_SUBSHELL_MODE);
		lexerState.deprecated_subshell_nesting_depths.push(0);
		lexerState.deprecated_subshell_nesting_depth = 0;
		emit(DEPRECATED_SUBSHELL_START, 0);
	}

	{AT} {
		/* Start of an interpolation */
		modeStack.push(BPP_REFERENCE_MODE);
		emit(AT);
	}

	\*/"@(" {
		// Asterisk followed by a supershell
		// Not a dereference operator, just a literal '*'
		// Supershells cannot be dereferenced
		emit(STRING_CONTENT, "*");
	}

	&/"@(" {
		// Ampersand followed by a supershell
		// Not an address-of operator, just a literal '&'
		// Supershells cannot have their address taken
		emit(STRING_CONTENT, "&");
	}

	\*/"@" {
		emit(DEREFERENCE_OPERATOR);
	}

	&/"@" {
		emit(AMPERSAND);
	}

	\* {
		emit(STRING_CONTENT, tokenText);
	}

	& {
		emit(STRING_CONTENT, tokenText);
	}
}

<DOUBLEQUOTE_MODE>{
	[^"@\\&\*\$`]+ {
		/* Consume all characters until closing doublequote or interpolation */
		emit(STRING_CONTENT, tokenText);
	}

	["] {
		/* Closing doublequote found */
		modeStack.pop(); // Exit DOUBLEQUOTE_MODE
		emit(QUOTE_END);
	}
}

<HEREDOC_CONTENT_MODE>{
	^[a-zA-Z_][a-zA-Z_0-9]*$ {
		// The heredoc delimiter must be at the start of the line
		// And it must be the only thing on the line
		std::string id = tokenText;
		if (lexerState.heredoc_delimiter == id) {
			// End of heredoc content
			modeStack.pop(); // Exit HEREDOC_CONTENT_MODE
			lexerState.heredoc_delimiter.clear();
			modeStack.pop(); // Exit HEREDOC_MODE
			emit(HEREDOC_END, id);
		} else {
			emit(STRING_CONTENT, tokenText);
		}
	}

	"\n" {
		emit(STRING_CONTENT, tokenText);
	}

	[a-zA-Z_][a-zA-Z_0-9]* {
		emit(STRING_CONTENT, tokenText);
	}

	[^a-zA-Z_0-9@\*&\n\$`\\]+ {
		emit(STRING_CONTENT, tokenText);
	}
}

<BASH_CASE_MODE>{
	"in"/[ \t\n] {
		if (lexerState.bash_case_input_received) {
			// This is only the special 'in' KEYWORD if we've received input for the case already
			lexerState.bash_case_keyword_in_received = true;
			emit(BASH_KEYWORD_IN);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	[ \t]+\n {
		if (lexerState.bash_case_keyword_in_received) {
			// Beginning of the case body
			modeStack.pop(); // Exit BASH_CASE_MODE
			modeStack.push(BASH_CASE_BODY_MODE);
			lexerState.bash_case_keyword_in_received = false; // Reset for next time
			lexerState.bash_case_input_received = false; // Reset for next time
			emit(BASH_CASE_BODY_BEGIN, tokenText);
		} else {
			emit(WS, tokenText);
		}
	}
}

<BASH_CASE_BODY_MODE>{
	[ \t\n]+esac/[^a-zA-Z0-9_] {
		modeStack.pop(); // Exit BASH_CASE_BODY_MODE
		emit(BASH_KEYWORD_ESAC);
	}

	[^\)@\\&\*\$`esac]+ {
		// Regular string content inside the case pattern
		emit(STRING_CONTENT, tokenText);
	}

	[esac]+ {
		// 'esac' fragment, but not the full 'esac' keyword
		emit(STRING_CONTENT, tokenText);
	}

	")" {
		// Pattern ends with ')'
		modeStack.push(BASH_CASE_PATTERN_ACTION_MODE); // Next up is the action following this pattern
		emit(BASH_CASE_PATTERN_DELIM);
	}
}

<BASH_CASE_PATTERN_ACTION_MODE>{
	";;" {
		// End of this pattern's action; back to parsing more patterns
		modeStack.pop(); // Exit BASH_CASE_PATTERN_ACTION_MODE
		emit(BASH_CASE_PATTERN_TERMINATOR);
	}
}

<BASH_FOR_OR_SELECT_MODE>{
	"in"/[^a-zA-Z0-9_] {
		if (lexerState.bash_for_or_select_variable_received) {
			// This is only the special 'in' KEYWORD if we've received the select variable already
			modeStack.pop(); // Exit BASH_FOR_OR_SELECT_MODE
			modeStack.push(BASH_FOR_OR_SELECT_INPUT_MODE);
			emit(BASH_KEYWORD_IN);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	"((" { emit(ARITH_FOR_CONDITION_START); }
	"))" { lexerState.bash_for_or_select_variable_received = true; emit(ARITH_FOR_CONDITION_END); }
	"++" { emit(INCREMENT_OPERATOR); }
	"--" { emit(DECREMENT_OPERATOR); }

	[0-9]+ { emit(INTEGER, tokenText); }

	"<=" { emit(COMPARISON_OPERATOR, tokenText); }
	">=" { emit(COMPARISON_OPERATOR, tokenText); }
	"==" { emit(COMPARISON_OPERATOR, tokenText); }
	"!=" { emit(COMPARISON_OPERATOR, tokenText); }
}

<BASH_FOR_OR_SELECT_BODY_MODE>{
	[ \t\n]+done/[^a-zA-Z0-9_] {
		modeStack.pop(); // Exit BASH_FOR_OR_SELECT_BODY_MODE
		emit(BASH_KEYWORD_DONE);
	}
}

<BPP_REFERENCE_MODE>{
	{IDENTIFIER} {
		emit(IDENTIFIER, tokenText);
	}

	"\." {
		emit(DOT);
	}

	(.|\n) {
		modeStack.pop(); // Exit BPP_REFERENCE_MODE if we see anything other than IDENTIFIER or DOT coming up next
		yyless(0); // Do not consume the character; let the main lexer mode handle it
	}
}

<BPP_BRACEREFERENCE_MODE>{
	"this" {
		// If the 'this' is the very FIRST token inside the brace reference, it's the 'this' keyword
		// Otherwise, it's just an identifier
		if (lexerState.token_count_inside_bracereference++ == 0) {
			emit(KEYWORD_THIS);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	"super" {
		// Likewise
		if (lexerState.token_count_inside_bracereference++ == 0) {
			emit(KEYWORD_SUPER);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	{IDENTIFIER} {
		lexerState.token_count_inside_bracereference++;
		emit(IDENTIFIER, tokenText);
	}

	"#" {
		// Don't count '#' towards token count
		// E.g., @{#this.array[@]}
		emit(HASH);
	}

	"\." {
		lexerState.token_count_inside_bracereference++;
		emit(DOT);
	}

	"[" {
		lexerState.token_count_inside_bracereference++;
		modeStack.push(ARRAY_INDEX_MODE);
		emit(ARRAY_INDEX_START);
	}

	"}" {
		modeStack.pop(); // Exit BPP_BRACEREFERENCE_MODE
		lexerState.token_count_inside_bracereference = 0;
		emit(REF_END);
	}
}

<BASH_VAR_MODE>{
	{IDENTIFIER} {
		emit(IDENTIFIER, tokenText);
	}

	"#" {
		emit(HASH);
	}
	
	"[" {
		modeStack.push(ARRAY_INDEX_MODE);
		emit(ARRAY_INDEX_START);
	}

	"}" {
		modeStack.pop(); // Exit BASH_VAR_MODE
		emit(BASH_VAR_END);
	}
}

<BPP_INCLUDE_MODE>{
	{WS} {
		emit(WS, tokenText);
	}
	{SPECIAL_KEYWORD_DYNAMIC} {
		emit(INCLUDE_TYPE, "dynamic");
	}
	{SPECIAL_KEYWORD_STATIC} {
		emit(INCLUDE_TYPE, "static");
	}
	{SPECIAL_KEYWORD_AS} {
		emit(KEYWORD_AS);
	}
	{QUOTED_INCLUDE_PATH} {
		emit(INCLUDE_PATH, tokenText);
	}
	{ANGLEBRACKET_INCLUDE_PATH} {
		emit(INCLUDE_PATH, tokenText);
	}
	[\n] {
		modeStack.pop(); // Exit BPP_INCLUDE_MODE
		emit(DELIM, tokenText);
	}
}

<*>. {
	emit(ERROR, tokenText);
}

%%

extern void scanner_push_state(int state, yyscan_t yyscanner) {
	yy_push_state(state, yyscanner);
}

extern void scanner_pop_state(yyscan_t yyscanner) {
	yy_pop_state(yyscanner);
}

extern int scanner_current_state(yyscan_t yyscanner) {
	struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
	return YY_START;
}

extern void destroyLexer() {
	modeStack.clear();
	lexerState.reset();
	yylex_destroy(main_lexer);
}
