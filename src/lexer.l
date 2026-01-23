

%{
/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */
#include "parser.tab.hpp" // Bison header for token types
#include "../AST/Token.h" // For AST::Token<T>
#include "../ModeStack.h"
#include <cstdint>
#include <stack>
#include <deque>

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

#define YY_DECL yy::parser::symbol_type yylex_impl(yyscan_t yyscanner)
YY_DECL;

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
	uint32_t if_depth = 0;
	uint32_t singlequote_start_line = 0;
	uint32_t singlequote_start_column = 0;
	std::stack<uint32_t> deprecated_subshell_nesting_depths;
	std::deque<std::string> heredoc_delimiters;
	bool bash_case_input_received = false;
	bool bash_case_keyword_in_received = false;
	bool bash_for_or_select_variable_received = false;
	bool bash_for_or_select_input_received = false;
	bool bash_if_condition_received = false;
	bool bash_while_or_until_condition_received = false;
	bool expecting_assignment_operator = false;
	bool currently_lexing_lvalue_reference = false;
	bool might_be_lexing_object_instantiation = false;
	bool parsed_assignment_operator = false;
	bool received_local_keyword = false;
	bool incoming_token_can_be_lvalue = true;

	yy::position current_position;
	yy::position token_beginning;
	yy::position token_ending;
	bool utf16_mode = false;


	void reset() {
		singlequote_contents.clear();
		singlequote_start_line = 0;
		singlequote_start_column = 0;
		token_count_inside_bracereference = 0;
		deprecated_subshell_nesting_depth = 0;
		if_depth = 0;
		while (!deprecated_subshell_nesting_depths.empty()) {
			deprecated_subshell_nesting_depths.pop();
		}
		heredoc_delimiters.clear();
		bash_case_input_received = false;
		bash_case_keyword_in_received = false;
		bash_for_or_select_variable_received = false;
		bash_for_or_select_input_received = false;
		bash_if_condition_received = false;
		bash_while_or_until_condition_received = false;
		expecting_assignment_operator = false;
		currently_lexing_lvalue_reference = false;
		might_be_lexing_object_instantiation = false;
		parsed_assignment_operator = false;
		received_local_keyword = false;
		incoming_token_can_be_lvalue = true;

		current_position.initialize();
		token_beginning.initialize();
		token_ending.initialize();
		utf16_mode = false;
	}
};

struct LexerExtra {
	LexerState lexerState;
	ModeStack modeStack;
	bool display_lexer_output = false;
};

extern LexerExtra* yyget_extra(yyscan_t yyscanner);
extern void yyset_extra(LexerExtra* extra, yyscan_t yyscanner);

static inline LexerExtra* get_lexer_extra(yyscan_t yyscanner) {
	return static_cast<LexerExtra*>(yyget_extra(yyscanner));
}

/* For compatibility with Bison's expectations, as documented above */
yy::parser::symbol_type yylex(yyscan_t yyscanner) {
	auto sym = yylex_impl(yyscanner);
	auto* extra = get_lexer_extra(yyscanner);
	if (extra && extra->display_lexer_output) {
		std::cout << "Token: " << yy::parser::symbol_name(sym.kind());
		
		std::string semantic_value = sym.value.as<std::string>();
		if (!semantic_value.empty()) {
			std::cout << ", Text: " << semantic_value;
		}

		std::cout << std::endl;
	}
	return sym;
}

void set_display_lexer_output(bool enable, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->display_lexer_output = enable;
}

size_t utf8_char_count(const std::string& s) {
	size_t count = 0;
	for (unsigned char c : s) {
		if ((c & 0xC0) != 0x80) {
			count++; // Skip UTF-8 continuation bytes
		}
	}
	return count;
}

// This function exists **solely** to accommodate the LSP.
size_t utf16_char_count(const std::string& s) {
	size_t count = 0;

	for (size_t i = 0; i < s.size();) {
		unsigned char c = s[i];
		uint32_t codepoint;
		size_t len;

		if ((c & 0x80) == 0) {
			// 1-byte character (ASCII)
			codepoint = c;
			len = 1;
		} else if ((c & 0xE0) == 0xC0) {
			// 2-byte character
			codepoint = (c & 0x1F) << 6 | (s[i + 1] & 0x3F);
			len = 2;
		} else if ((c & 0xF0) == 0xE0) {
			// 3-byte character
			codepoint = (c & 0x0F) << 12 | (s[i + 1] & 0x3F) << 6 | (s[i + 2] & 0x3F);
			len = 3;
		} else if ((c & 0xF8) == 0xF0) {
			// 4-byte character
			codepoint = (c & 0x07) << 18 | (s[i + 1] & 0x3F) << 12 | (s[i + 2] & 0x3F) << 6 | (s[i + 3] & 0x3F);
			len = 4;
		} else {
			// Invalid UTF-8 sequence, treat as a single character
			codepoint = c;
			len = 1;
		}

		if (codepoint >= 0x10000) {
			// Convert to UTF-16 surrogate pair
			count += 2; // Surrogate pairs count as two UTF-16 code units
		} else {
			count++; // Single UTF-16 code unit
		}

		i += len;
	}

	return count;
}

size_t char_count(const std::string& s, yyscan_t yyscanner) {
	if (get_lexer_extra(yyscanner)->lexerState.utf16_mode) {
		return utf16_char_count(s);
	}

	return utf8_char_count(s);
}

void advance_position(const std::string& text, yyscan_t yyscanner) {
	std::string line;
	for (char c : text) {
		switch (c) {
			case '\n':
				get_lexer_extra(yyscanner)->lexerState.current_position.line++;
				get_lexer_extra(yyscanner)->lexerState.current_position.column = 1;
				line.clear();
				break;
			case '\r':
				// Ignore carriage returns
				break;
			default:
				line += c;
				break;
		}
	}
	get_lexer_extra(yyscanner)->lexerState.current_position.columns(char_count(line, yyscanner));
}

void rewind_char_counter_to_match_beginning(yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.current_position = get_lexer_extra(yyscanner)->lexerState.token_beginning;
	get_lexer_extra(yyscanner)->lexerState.token_ending = get_lexer_extra(yyscanner)->lexerState.token_beginning;
}

void char_counter_zero_length_match_here(yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.token_beginning = get_lexer_extra(yyscanner)->lexerState.current_position;
	get_lexer_extra(yyscanner)->lexerState.token_ending = get_lexer_extra(yyscanner)->lexerState.current_position;
}

yy::location current_match_location(yyscan_t yyscanner) {
	yy::location result;
	result.begin = get_lexer_extra(yyscanner)->lexerState.token_beginning;
	result.end = get_lexer_extra(yyscanner)->lexerState.token_ending;
	return result;
}

#define reconsume_token() \
	rewind_char_counter_to_match_beginning(yyscanner); yyless(0);

// YY_USER_ACTION runs for every match
// Any time we call yyless(0), we have to remember to call rewind_char_counter_to_match_beginning()
// Therefore we do not call yyless(0), we call the reconsume_token() macro
#define YY_USER_ACTION do { \
	get_lexer_extra(yyscanner)->lexerState.token_beginning = get_lexer_extra(yyscanner)->lexerState.current_position; \
	advance_position(std::string(yytext, yyleng), yyscanner); \
	get_lexer_extra(yyscanner)->lexerState.token_ending = get_lexer_extra(yyscanner)->lexerState.current_position; \
} while (false);

#define yyterminate() return yy::parser::make_YYEOF(current_match_location(yyscanner));

/* For external use by the parser
 * After the parser parses an assignment statement,
 * It can use this function to signal back to the lexer
 * That the next token could be an lvalue
 */
void set_incoming_token_can_be_lvalue(bool canBeLvalue, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = canBeLvalue;
}

void set_bash_case_input_received(bool received, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.bash_case_input_received = received;
}

void set_bash_for_or_select_variable_received(bool received, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_variable_received = received;
}

void set_bash_if_condition_received(bool received, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.bash_if_condition_received = received;
}

void set_bash_while_or_until_condition_received(bool received, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.bash_while_or_until_condition_received = received;
}

void set_parsed_assignment_operator(bool parsed, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.parsed_assignment_operator = parsed;
}

void set_received_local_keyword(bool received, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.received_local_keyword = received;
}

/**
 * @brief Determines if the current token can be treated as an lvalue, and updates lexer state accordingly.
 *
 * @param token The current token being processed.
 * @return Either the original token unmodified, or an lvalue variant of the token if applicable.
*/
yy::parser::symbol_type maybe_get_lvalue_token(yy::parser::symbol_type token, yyscan_t yyscanner);

void updateLexerState(yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = false;
	get_lexer_extra(yyscanner)->lexerState.parsed_assignment_operator = false;
}

%}

%option noyywrap
%option stack
%option reentrant
%option extra-type="LexerExtra*"

ESCAPED_CHAR    \\(.|\n)
AT              @

WS              [ \t]+

DELIM           [ \t]*[\n;]

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

%x UNLEXED_PARAMETER_EXPANSION_MODE

%x HEREDOC_CONTENT_MODE

%x BASH_CASE_BODY_MODE

%x BASH_FUNCTION_MODE

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

%s BPP_CLASS_HEADER_MODE

%s ARRAY_INDEX_MODE

%s SUPERSHELL_MODE

%s SUBSHELL_MODE

%s SUBSHELL_SUBSTITUTION_MODE

%s DEPRECATED_SUBSHELL_MODE

%s PROCESS_SUBSTITUTION_MODE

%s BLOCK_MODE

%s PARAMETER_EXPANSION_MODE

%s HEREDOC_MODE

%s BASH_CASE_MODE

%s BASH_CASE_PATTERN_ACTION_MODE

%s BASH_FOR_OR_SELECT_MODE

%s BASH_FOR_OR_SELECT_INPUT_MODE

%s BASH_FOR_OR_SELECT_BODY_MODE

%s BASH_WHILE_OR_UNTIL_BODY_MODE

%s BASH_IF_MODE

%s BASH_ARITHMETIC_MODE

%s BASH_TEST_CONDITION_MODE

%s BASH_53_NATIVE_SUPERSHELL_MODE

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
		updateLexerState(yyscanner); \
		return maybe_get_lvalue_token(yy::parser::make_##tokenType( \
			__VA_ARGS__ __VA_OPT__(,) current_match_location(yyscanner) \
		), yyscanner);
	
	/**
	* Helper to get the text of the current token as a std::string.
	*/
	#define tokenText AST::Token<std::string>(std::string(yytext, yyleng), get_lexer_extra(yyscanner)->lexerState.token_beginning.line, get_lexer_extra(yyscanner)->lexerState.token_beginning.column)
%}

<*>{
	[\r]+ { /* Ignore carriage returns globally */ }
}

<SKIP_AFTER_DELIM_MODE>{
	[ \t\n]+    { /* Ignore whitespace and newlines after a DELIM */ }
	[\#][^\n]*  { /* Ignore comments */ }
	.           {
		// Any other character means we're done skipping
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit SKIP_AFTER_DELIM_MODE
		reconsume_token();
	}
}

{ESCAPED_CHAR}                       {
	std::string sequence = tokenText;
	if (get_lexer_extra(yyscanner)->modeStack.top() == DEPRECATED_SUBSHELL_MODE && sequence[1] == '`') {
		// Is this starting a nested subshell or is it finishing a nested subshell?
		get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth++; // Include the latest backslash
		if (
			!get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.empty()
			&& get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth == get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.top()
		) {
			// Finishing a nested subshell
			get_lexer_extra(yyscanner)->modeStack.pop();
			uint32_t depth = get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth;
			get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.pop();
			get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth = 0;
			emit(DEPRECATED_SUBSHELL_END, depth);
		} else {
			// Starting a new nested subshell
			get_lexer_extra(yyscanner)->modeStack.push(DEPRECATED_SUBSHELL_MODE);
			get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.push(get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth);
			get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth = 0;
			emit(DEPRECATED_SUBSHELL_START, get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.top());
		}
	} else if (get_lexer_extra(yyscanner)->modeStack.top() == DEPRECATED_SUBSHELL_MODE && sequence[1] == '\\') {
		get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth++;
		// No need to emit anything yet
	} else {
		emit(CATCHALL, tokenText);
	}
}
{WS}                                 {
	if (get_lexer_extra(yyscanner)->lexerState.parsed_assignment_operator) {
		// If whitespace immediately follows an assignment operator,
		// The rvalue of the assignment is empty
		reconsume_token();
		char_counter_zero_length_match_here(yyscanner);
		emit(EMPTY_ASSIGNMENT);
	}
	emit(WS, tokenText);
}

{AT}                                 { get_lexer_extra(yyscanner)->modeStack.push(BPP_MODE); reconsume_token(); }

<BPP_MODE>{
	{KEYWORD_INCLUDE}/[^a-zA-Z0-9_]      { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_INCLUDE_MODE); emit(KEYWORD_INCLUDE); }
	{KEYWORD_INCLUDE_ONCE}/[^a-zA-Z0-9_] { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_INCLUDE_MODE); emit(KEYWORD_INCLUDE_ONCE); }

	{KEYWORD_CLASS}/[^a-zA-Z0-9_]        { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_CLASS_HEADER_MODE); emit(KEYWORD_CLASS); }
	{KEYWORD_PUBLIC}/[^a-zA-Z0-9_]       { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_PUBLIC); }
	{KEYWORD_PROTECTED}/[^a-zA-Z0-9_]    { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_PROTECTED); }
	{KEYWORD_PRIVATE}/[^a-zA-Z0-9_]      { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_PRIVATE); }
	{KEYWORD_NEW}/[^a-zA-Z0-9_]          { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_NEW); }
	{KEYWORD_DELETE}/[^a-zA-Z0-9_]       { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_DELETE); }
	{KEYWORD_NULLPTR}/[^a-zA-Z0-9_]      { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_NULLPTR); }
	{KEYWORD_METHOD}/[^a-zA-Z0-9_]       { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_METHOD); }
	{KEYWORD_VIRTUAL}/[^a-zA-Z0-9_]      { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_VIRTUAL); }
	{KEYWORD_CONSTRUCTOR}/[^a-zA-Z0-9_]  { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_CONSTRUCTOR); }
	{KEYWORD_DESTRUCTOR}/[^a-zA-Z0-9_]   { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_DESTRUCTOR); }
	{KEYWORD_DYNAMIC_CAST}/[^a-zA-Z0-9_] { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_DYNAMIC_CAST); }
	{KEYWORD_TYPEOF}/[^a-zA-Z0-9_]       { get_lexer_extra(yyscanner)->modeStack.pop(); emit(KEYWORD_TYPEOF); }
	{KEYWORD_THIS}/[^a-zA-Z0-9_]         { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_REFERENCE_MODE); emit(KEYWORD_THIS); }
	{KEYWORD_SUPER}/[^a-zA-Z0-9_]        { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_REFERENCE_MODE); emit(KEYWORD_SUPER); }

	"@{"                                 { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_BRACEREFERENCE_MODE); emit(REF_START); }
	"@("                                 { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(SUPERSHELL_MODE); emit(SUPERSHELL_START); }
	{AT}                                 { get_lexer_extra(yyscanner)->modeStack.pop(); get_lexer_extra(yyscanner)->modeStack.push(BPP_REFERENCE_MODE); emit(AT); }
}

<BPP_CLASS_HEADER_MODE>{
	":" { get_lexer_extra(yyscanner)->modeStack.pop(); emit(COLON); }
}

{DELIM}                              {
	switch (get_lexer_extra(yyscanner)->modeStack.top()) {
		case BASH_CASE_MODE:
			if (get_lexer_extra(yyscanner)->lexerState.bash_case_keyword_in_received) {
				// Beginning of the case body
				get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_CASE_MODE
				get_lexer_extra(yyscanner)->modeStack.push(BASH_CASE_BODY_MODE);
				get_lexer_extra(yyscanner)->lexerState.bash_case_keyword_in_received = false; // Reset for next time
				get_lexer_extra(yyscanner)->lexerState.bash_case_input_received = false; // Reset for next time
				get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
				emit(BASH_CASE_BODY_BEGIN, tokenText); // NOTE: CHANGE THIS IF emit() MACRO CHANGES
				// We are VERY MUCH relying on emit() returning immediately
			} else {
				get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
				emit(WS, tokenText);
			}
			break;
		case BASH_FOR_OR_SELECT_INPUT_MODE:
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_FOR_OR_SELECT_INPUT_MODE
			get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_input_received = true; // So that we know 'do' can be a keyword
			get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_variable_received = false; // Reset for next time
			get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
			emit(DELIM, tokenText); // NOTE: CHANGE THIS IF emit() MACRO CHANGES
			// Same deal as above
			break;
		case BASH_FOR_OR_SELECT_MODE:
			if (get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_variable_received) {
				// No 'in' keyword; proceed to body directly
				get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_FOR_OR_SELECT_MODE
				get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_input_received = true; // So that we know 'do' can be a keyword
				get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_variable_received = false; // Reset for next time
				get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
				emit(DELIM, tokenText); // NOTE: CHANGE THIS IF emit() MACRO CHANGES
			}
			break;
		default:
			if (!get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters.empty() && tokenText == "\n") {
				// Do not push SKIP_AFTER_DELIM_MODE if we're going into a string-type mode like HEREDOC_CONTENT_MODE
				for (const auto& delimiter : get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters) {
					// Commands can contain multiple heredocs, as in:
					// cat <<EOF1 <<EOF2 <<EOF3
					// These heredocs must then be terminated in the order they're given
					// I.e., first EOF1, then EOF2, then EOF3
					// So, we push HEREDOC_CONTENT_MODE for each of the given delimiters
					// And pop one off for each that we finish
					get_lexer_extra(yyscanner)->modeStack.push(HEREDOC_CONTENT_MODE); 
				}
				emit(HEREDOC_CONTENT_START);
			}
			break;
	}

	if (get_lexer_extra(yyscanner)->lexerState.parsed_assignment_operator) {
		// If a DELIM immediately follows an assignment operator,
		// The rvalue of the assignment is empty
		reconsume_token();
		char_counter_zero_length_match_here(yyscanner);
		// Do not push SKIP_AFTER_DELIM_MODE yet, as we need to reconsume the DELIM first
		emit(EMPTY_ASSIGNMENT);
	}
	get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
	emit(DELIM, tokenText);
}
[ \t]*&&                             { emit(DOUBLEAMPERSAND); }
[ \t]*\|\|                           { emit(DOUBLEPIPE); }
[ \t]*\|                             { emit(PIPE); }
{LBRACE}                             { get_lexer_extra(yyscanner)->modeStack.push(BLOCK_MODE); emit(LBRACE); }
<BLOCK_MODE>{
	{RBRACE} {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BLOCK_MODE
		emit(RBRACE);
	}
}
<BASH_53_NATIVE_SUPERSHELL_MODE>{
	{RBRACE} {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_53_NATIVE_SUPERSHELL_MODE
		emit(BASH_53_NATIVE_SUPERSHELL_END);
	}
}
"+="                                 {
	if (get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator) {
		get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = false;
		emit(PLUS_EQUALS);
	} else {
		emit(CATCHALL, tokenText);
	}
}
"="                                  {
	if (get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator) {
		get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = false;
		emit(EQUALS);
	} else {
		emit(CATCHALL, tokenText);
	}
}
\{[0-9]+\.\.[0-9]+\}                 { emit(CATCHALL, tokenText); /* Range expression, e.g., {1..10} */ }
\{[0-9]+\.\.[0-9]+\.\.[0-9]+\}       { emit(CATCHALL, tokenText); /* Range expression with step, e.g., {1..10..2} */ }
[\*]/[@]                             { emit(DEREFERENCE_OPERATOR); }
[&]/[@]                              { emit(AMPERSAND); }

<BASH_FUNCTION_MODE>{
	[ \t\n]+ { /* Ignore whitespace in this mode */ }

	[a-zA-Z_][a-zA-Z_0-9]* {
		emit(BASH_FUNCTION_LABEL, tokenText);
	}

	[ \t\n]*\([ \t\n]*\) {
		emit(BASH_FUNCTION_OPEN);
	}

	{LBRACE} {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_FUNCTION_MODE
		get_lexer_extra(yyscanner)->modeStack.push(BLOCK_MODE);
		emit(LBRACE);
	}
}

function/[ \t]+[a-zA-Z_][a-zA-Z_0-9]* {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
		get_lexer_extra(yyscanner)->modeStack.push(BASH_FUNCTION_MODE);
		emit(BASH_KEYWORD_FUNCTION);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

[a-zA-Z_][a-zA-Z_0-9]*/[ \t]*\([ \t]*\) {
	// This matches a function declaration without the 'function' keyword
	// The meaning of the regular expression is:
	// An identifier, iff that identifier is followed by ()
	// Where whitespace may precede the LPAREN, and whitespace may be between the parentheses
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
		get_lexer_extra(yyscanner)->modeStack.push(BASH_FUNCTION_MODE);
		emit(BASH_FUNCTION_LABEL, tokenText);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

{IDENTIFIER}                         {
	if (get_lexer_extra(yyscanner)->modeStack.top() == HEREDOC_MODE) {
		// This IDENTIFIER is the heredoc delimiter
		get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters.push_back(tokenText);
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit HEREDOC_MODE
		emit(HEREDOC_DELIMITER, tokenText);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"<("                                      { get_lexer_extra(yyscanner)->modeStack.push(PROCESS_SUBSTITUTION_MODE); emit(PROCESS_SUBSTITUTION_START, tokenText); }
">("                                      { get_lexer_extra(yyscanner)->modeStack.push(PROCESS_SUBSTITUTION_MODE); emit(PROCESS_SUBSTITUTION_START, tokenText); }
([0-9]+|\{[a-zA-Z_][a-zA-Z_0-9]*\})?\<    { emit(LANGLE, tokenText); }
([0-9]+|\{[a-zA-Z_][a-zA-Z_0-9]*\})?\>    { emit(RANGLE, tokenText); }
([0-9]+|\{[a-zA-Z_][a-zA-Z_0-9]*\})?\&\>  { emit(AMPERSAND_RANGLE, tokenText); }
([0-9]+|\{[a-zA-Z_][a-zA-Z_0-9]*\})?\>\&  { emit(RANGLE_AMPERSAND, tokenText); }
([0-9]+|\{[a-zA-Z_][a-zA-Z_0-9]*\})?\<\&  { emit(LANGLE_AMPERSAND, tokenText); }

"${"                                 { get_lexer_extra(yyscanner)->modeStack.push(BASH_VAR_MODE); emit(BASH_VAR_START); }

\${IDENTIFIER}                       { emit(BASH_VAR, tokenText); }
\$[\@\#\*\-\$\!\?]                   { emit(BASH_VAR, tokenText); }
\$\{[\@\#\*\-\$!\?]\}                { emit(BASH_VAR, tokenText); }
\${INTEGER}                          { emit(BASH_VAR, tokenText); }
\$\{{INTEGER}\}                      { emit(BASH_VAR, tokenText); }

"]"                                  {
	switch (get_lexer_extra(yyscanner)->modeStack.top()) {
		case ARRAY_INDEX_MODE:
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit ARRAY_INDEX_MODE
			emit(ARRAY_INDEX_END);
			break;
		default:
			emit(CATCHALL, tokenText);
			break;
	}
}

\$\(\([ \t]* {
	get_lexer_extra(yyscanner)->modeStack.push(BASH_ARITHMETIC_MODE);
	emit(BASH_ARITHMETIC_START);
}

<BASH_ARITHMETIC_MODE>{
	[ \t]*\)\) {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_ARITHMETIC_MODE
		emit(BASH_ARITHMETIC_END);
	}
}

"$(" {
	get_lexer_extra(yyscanner)->modeStack.push(SUBSHELL_SUBSTITUTION_MODE);
	emit(SUBSHELL_SUBSTITUTION_START);
}

"(" {
	get_lexer_extra(yyscanner)->modeStack.push(SUBSHELL_MODE);
	emit(SUBSHELL_START);
}

\[\[[ \t]+ {
	// Matches the opening '[[' of a bash test condition
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
		get_lexer_extra(yyscanner)->modeStack.push(BASH_TEST_CONDITION_MODE);
		emit(BASH_TEST_CONDITION_START);
	} else {
		// If it's not an lvalue token, it's just normal text.
		emit(CATCHALL, tokenText);
	}
}

<BASH_TEST_CONDITION_MODE>{
	[ \t]*\]\]/[ \t\n;]+ {
		// Matches the closing ']]'
		get_lexer_extra(yyscanner)->modeStack.pop();
		emit(BASH_TEST_CONDITION_END);
	}
}

<SUPERSHELL_MODE,SUBSHELL_MODE,SUBSHELL_SUBSTITUTION_MODE,PROCESS_SUBSTITUTION_MODE>{
	[ \t]*\) {
		// Absorb preceding whitespace
		auto current_mode = get_lexer_extra(yyscanner)->modeStack.top();
		get_lexer_extra(yyscanner)->modeStack.pop();
		switch (current_mode) {
			case SUPERSHELL_MODE:
				emit(SUPERSHELL_END);
				break;
			case SUBSHELL_MODE:
				emit(SUBSHELL_END);
				break;
			case SUBSHELL_SUBSTITUTION_MODE:
				emit(SUBSHELL_SUBSTITUTION_END);
				break;
			case PROCESS_SUBSTITUTION_MODE:
				emit(PROCESS_SUBSTITUTION_END);
				break;
		}
	}
}

")" { emit(RPAREN); }

"`" {
	switch (get_lexer_extra(yyscanner)->modeStack.top()) {
		case DEPRECATED_SUBSHELL_MODE:
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit DEPRECATED_SUBSHELL_MODE
			emit(DEPRECATED_SUBSHELL_END, 0);
			break;
		default:
			get_lexer_extra(yyscanner)->modeStack.push(DEPRECATED_SUBSHELL_MODE);
			get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.push(0);
			get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth = 0;
			emit(DEPRECATED_SUBSHELL_START, 0);
			break;
	}
}

['] {
	get_lexer_extra(yyscanner)->modeStack.push(SINGLEQUOTE_MODE);
	get_lexer_extra(yyscanner)->lexerState.singlequote_contents.clear();
	get_lexer_extra(yyscanner)->lexerState.singlequote_contents += '\'';
	get_lexer_extra(yyscanner)->lexerState.singlequote_start_line = get_lexer_extra(yyscanner)->lexerState.token_beginning.line;
	get_lexer_extra(yyscanner)->lexerState.singlequote_start_column = get_lexer_extra(yyscanner)->lexerState.token_beginning.column;
}

["] {
	get_lexer_extra(yyscanner)->modeStack.push(DOUBLEQUOTE_MODE);
	emit(QUOTE_BEGIN);
}

"<<-" {
	get_lexer_extra(yyscanner)->modeStack.push(HEREDOC_MODE);
	emit(HEREDOC_START, tokenText);
}

"<<<" { emit(HERESTRING_START); }

"<<" {
	get_lexer_extra(yyscanner)->modeStack.push(HEREDOC_MODE);
	emit(HEREDOC_START, tokenText);
}

"case"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
		get_lexer_extra(yyscanner)->modeStack.push(BASH_CASE_MODE);
		emit(BASH_KEYWORD_CASE);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"select"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
		get_lexer_extra(yyscanner)->modeStack.push(BASH_FOR_OR_SELECT_MODE);
		emit(BASH_KEYWORD_SELECT);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"for"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
		get_lexer_extra(yyscanner)->modeStack.push(BASH_FOR_OR_SELECT_MODE);
		emit(BASH_KEYWORD_FOR);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"if"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		// The next token can and should be an lvalue
		get_lexer_extra(yyscanner)->modeStack.push(BASH_IF_MODE);
		get_lexer_extra(yyscanner)->lexerState.if_depth++;
		emit(BASH_KEYWORD_IF);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"fi"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.if_depth > 0) {
		get_lexer_extra(yyscanner)->lexerState.if_depth--;
		emit(BASH_KEYWORD_FI);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"elif"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.if_depth > 0) {
		// The next token can and should be an lvalue
		get_lexer_extra(yyscanner)->modeStack.push(BASH_IF_MODE);
		emit(BASH_KEYWORD_ELIF);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"else"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.if_depth > 0) {
		emit(BASH_KEYWORD_ELSE);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"while"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		// The next token can and should be an lvalue
		emit(BASH_KEYWORD_WHILE);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"until"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
		// The next token can and should be an lvalue
		emit(BASH_KEYWORD_UNTIL);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"do"/[ \t\n] {
	if (get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_input_received) {
		get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_input_received = false; // Reset for next time
		get_lexer_extra(yyscanner)->modeStack.push(BASH_FOR_OR_SELECT_BODY_MODE);
		emit(BASH_KEYWORD_DO);
	} else if (get_lexer_extra(yyscanner)->lexerState.bash_while_or_until_condition_received) {
		get_lexer_extra(yyscanner)->lexerState.bash_while_or_until_condition_received = false; // Reset for next time
		get_lexer_extra(yyscanner)->modeStack.push(BASH_WHILE_OR_UNTIL_BODY_MODE);
		emit(BASH_KEYWORD_DO);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

"local"/[ \t] {
	if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue
	&& get_lexer_extra(yyscanner)->lexerState.received_local_keyword == false
	) {
		emit(BASH_KEYWORD_LOCAL);
	} else {
		emit(IDENTIFIER, tokenText);
	}
}

{INTEGER} { emit(INTEGER, tokenText); }

[ \t]*[\#][^\n]*  { /* Ignore comments */ }

<BASH_IF_MODE>{
	"then"/[ \t\n] {
		if (get_lexer_extra(yyscanner)->lexerState.bash_if_condition_received) {
			get_lexer_extra(yyscanner)->lexerState.bash_if_condition_received = false; // Reset for next time
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_IF_MODE
			get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
			emit(BASH_KEYWORD_THEN);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}
}

<SINGLEQUOTE_MODE>{
	[^'\\]+ {
		/* Consume all characters until closing singlequote */
		get_lexer_extra(yyscanner)->lexerState.singlequote_contents += tokenText;
	}

	{ESCAPED_CHAR} {
		/* If it's an escaped singlequote, it's not the end of the string */
		get_lexer_extra(yyscanner)->lexerState.singlequote_contents += tokenText;
	}

	"'" {
		/* Closing singlequote found */
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit SINGLEQUOTE_MODE
		get_lexer_extra(yyscanner)->lexerState.singlequote_contents += '\'';
		uint32_t start_line = get_lexer_extra(yyscanner)->lexerState.singlequote_start_line;
		uint32_t start_column = get_lexer_extra(yyscanner)->lexerState.singlequote_start_column;
		get_lexer_extra(yyscanner)->lexerState.singlequote_start_line = 0;
		get_lexer_extra(yyscanner)->lexerState.singlequote_start_column = 0;
		emit(SINGLEQUOTED_STRING, AST::Token<std::string>(get_lexer_extra(yyscanner)->lexerState.singlequote_contents, start_line, start_column));
	}
}

<DOUBLEQUOTE_MODE,HEREDOC_CONTENT_MODE,BASH_CASE_BODY_MODE>{
	/* The following rules are for handling string interpolations */
	\\@ {
		// Literal '@'
		emit(STRING_CONTENT, AST::Token<std::string>("@", get_lexer_extra(yyscanner)->lexerState.token_beginning.line, get_lexer_extra(yyscanner)->lexerState.token_beginning.column));
	}

	\$@ {
		// Bash variable: '$@'
		emit(STRING_CONTENT, tokenText);
	}

	\\` {
		// Literal backtick
		emit(STRING_CONTENT, tokenText);
	}

	\\[\*&] {
		// Literal '*' or '&'
		emit(STRING_CONTENT, tokenText);
	}

	\\ {
		// Literal backslash
		emit(STRING_CONTENT, tokenText);
	}

	"@(" {
		get_lexer_extra(yyscanner)->modeStack.push(SUPERSHELL_MODE);
		emit(SUPERSHELL_START);
	}

	"@{" {
		get_lexer_extra(yyscanner)->modeStack.push(BPP_BRACEREFERENCE_MODE);
		emit(REF_START);
	}

	"$(" {
		get_lexer_extra(yyscanner)->modeStack.push(SUBSHELL_SUBSTITUTION_MODE);
		emit(SUBSHELL_SUBSTITUTION_START);
	}

	"${" {
		get_lexer_extra(yyscanner)->modeStack.push(BASH_VAR_MODE);
		emit(BASH_VAR_START);
	}

	"$" {
		// Solitary $
		emit(STRING_CONTENT, tokenText);
	}

	"`" {
		get_lexer_extra(yyscanner)->modeStack.push(DEPRECATED_SUBSHELL_MODE);
		get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depths.push(0);
		get_lexer_extra(yyscanner)->lexerState.deprecated_subshell_nesting_depth = 0;
		emit(DEPRECATED_SUBSHELL_START, 0);
	}

	{AT} {
		/* Start of an interpolation */
		get_lexer_extra(yyscanner)->modeStack.push(BPP_REFERENCE_MODE);
		emit(AT);
	}

	{KEYWORD_THIS}/[^a-zA-Z0-9_] {
		// Special case: @this
		get_lexer_extra(yyscanner)->modeStack.push(BPP_REFERENCE_MODE);
		emit(KEYWORD_THIS);
	}

	{KEYWORD_SUPER}/[^a-zA-Z0-9_] {
		// Special case: @super
		get_lexer_extra(yyscanner)->modeStack.push(BPP_REFERENCE_MODE);
		emit(KEYWORD_SUPER);
	}

	\*/"@(" {
		// Asterisk followed by a supershell
		// Not a dereference operator, just a literal '*'
		// Supershells cannot be dereferenced
		emit(STRING_CONTENT, tokenText);
	}

	&/"@(" {
		// Ampersand followed by a supershell
		// Not an address-of operator, just a literal '&'
		// Supershells cannot have their address taken
		emit(STRING_CONTENT, tokenText);
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

	\\["] {
		/* Escaped doublequote */
		emit(STRING_CONTENT, tokenText);
	}

	["] {
		/* Closing doublequote found */
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit DOUBLEQUOTE_MODE
		emit(QUOTE_END);
	}
}

<HEREDOC_CONTENT_MODE>{
	^[a-zA-Z_][a-zA-Z_0-9]*$ {
		// The heredoc delimiter must be at the start of the line
		// And it must be the only thing on the line
		AST::Token<std::string> id = tokenText;
		if (!get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters.empty() && get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters.front() == id.getValue()) {
			// End of heredoc content
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit HEREDOC_CONTENT_MODE
			get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters.pop_front();
			if (get_lexer_extra(yyscanner)->lexerState.heredoc_delimiters.empty()) {
				emit(HEREDOC_END, id);
			} else {
				// If the queue is not yet empty, the heredoc sequence is not yet over
				emit(STRING_CONTENT, id);
			}
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
		if (get_lexer_extra(yyscanner)->lexerState.bash_case_input_received) {
			// This is only the special 'in' KEYWORD if we've received input for the case already
			get_lexer_extra(yyscanner)->lexerState.bash_case_keyword_in_received = true;
			emit(BASH_KEYWORD_IN);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	[ \t]+\n {
		if (get_lexer_extra(yyscanner)->lexerState.bash_case_keyword_in_received) {
			// Beginning of the case body
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_CASE_MODE
			get_lexer_extra(yyscanner)->modeStack.push(BASH_CASE_BODY_MODE);
			get_lexer_extra(yyscanner)->lexerState.bash_case_keyword_in_received = false; // Reset for next time
			get_lexer_extra(yyscanner)->lexerState.bash_case_input_received = false; // Reset for next time
			emit(BASH_CASE_BODY_BEGIN, tokenText);
		} else {
			emit(WS, tokenText);
		}
	}
}

<BASH_CASE_BODY_MODE>{
	[ \t\n]+esac/[^a-zA-Z0-9_] {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_CASE_BODY_MODE
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
		get_lexer_extra(yyscanner)->modeStack.push(BASH_CASE_PATTERN_ACTION_MODE); // Next up is the action following this pattern
		get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the ')'
		emit(BASH_CASE_PATTERN_DELIM);
	}
}

<BASH_CASE_PATTERN_ACTION_MODE>{
	[ \t]*;; {
		// End of this pattern's action; back to parsing more patterns
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_CASE_PATTERN_ACTION_MODE
		emit(BASH_CASE_PATTERN_TERMINATOR);
	}
}

<BASH_FOR_OR_SELECT_MODE>{
	"in"/[^a-zA-Z0-9_] {
		if (get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_variable_received) {
			// This is only the special 'in' KEYWORD if we've received the select variable already
			get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_FOR_OR_SELECT_MODE
			get_lexer_extra(yyscanner)->modeStack.push(BASH_FOR_OR_SELECT_INPUT_MODE);
			emit(BASH_KEYWORD_IN);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	"((" { emit(ARITH_FOR_CONDITION_START); }
	"))" { get_lexer_extra(yyscanner)->lexerState.bash_for_or_select_variable_received = true; emit(ARITH_FOR_CONDITION_END); }
	"++" { emit(INCREMENT_OPERATOR); }
	"--" { emit(DECREMENT_OPERATOR); }

	"<=" { emit(COMPARISON_OPERATOR, tokenText); }
	">=" { emit(COMPARISON_OPERATOR, tokenText); }
	"==" { emit(COMPARISON_OPERATOR, tokenText); }
	"!=" { emit(COMPARISON_OPERATOR, tokenText); }
}

<BASH_FOR_OR_SELECT_BODY_MODE,BASH_WHILE_OR_UNTIL_BODY_MODE>{
	[ \t\n]*done/[^a-zA-Z0-9_] {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit the mode
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

	"\*" {
		emit(ASTERISK);
	}

	[ \t]+[a-zA-Z_][a-zA-Z_0-9]* {
		// We see up ahead: whitespace, and then another identifier
		get_lexer_extra(yyscanner)->modeStack.pop();
		if (get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference) {
			get_lexer_extra(yyscanner)->lexerState.might_be_lexing_object_instantiation = true;
		}
		reconsume_token();
	}

	(.|\n) {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BPP_REFERENCE_MODE if we see anything other than IDENTIFIER or DOT coming up next
		if (get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference) {
			get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = false;
			get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = true;
		}
		reconsume_token(); // Do not consume the character; let the main lexer mode handle it
	}
}

<BPP_BRACEREFERENCE_MODE>{
	"this" {
		// If the 'this' is the very FIRST token inside the brace reference, it's the 'this' keyword
		// Otherwise, it's just an identifier
		if (get_lexer_extra(yyscanner)->lexerState.token_count_inside_bracereference++ == 0) {
			emit(KEYWORD_THIS);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	"super" {
		// Likewise
		if (get_lexer_extra(yyscanner)->lexerState.token_count_inside_bracereference++ == 0) {
			emit(KEYWORD_SUPER);
		} else {
			emit(IDENTIFIER, tokenText);
		}
	}

	{IDENTIFIER} {
		get_lexer_extra(yyscanner)->lexerState.token_count_inside_bracereference++;
		emit(IDENTIFIER, tokenText);
	}

	"#" {
		// Don't count '#' towards token count
		// E.g., @{#this.array[@]}
		emit(HASH);
	}

	"\." {
		get_lexer_extra(yyscanner)->lexerState.token_count_inside_bracereference++;
		emit(DOT);
	}

	"[" {
		get_lexer_extra(yyscanner)->lexerState.token_count_inside_bracereference = 0; // Reset in case there's another reference inside the array index
		get_lexer_extra(yyscanner)->modeStack.push(ARRAY_INDEX_MODE);
		emit(ARRAY_INDEX_START);
	}

	"}" {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BPP_BRACEREFERENCE_MODE
		get_lexer_extra(yyscanner)->lexerState.token_count_inside_bracereference = 0;
		if (get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference) {
			get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = false;
			get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = true;
		}
		emit(REF_END);
	}
}

<BASH_VAR_MODE>{
	{IDENTIFIER} {
		emit(IDENTIFIER, tokenText);
	}

	{WS} {
		// Replace BASH_VAR_MODE with BASH_53_NATIVE_SUPERSHELL_MODE
		// Bash 5.3's native supershell syntax is roughly ${ statements ; }
		// Equivalent to our original @(statements)
		// (Make no mistake, this was originally developed & released in Bash++, and was added to Bash a full year later)
		get_lexer_extra(yyscanner)->modeStack.pop();
		get_lexer_extra(yyscanner)->modeStack.push(BASH_53_NATIVE_SUPERSHELL_MODE);
		get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = false;
		get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = true;
		emit(WS, tokenText);
	}

	"#" {
		emit(HASH);
	}
	
	"[" {
		get_lexer_extra(yyscanner)->modeStack.push(ARRAY_INDEX_MODE);
		emit(ARRAY_INDEX_START);
	}

	"!" {
		emit(EXCLAM);
	}

	":" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"@" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"%" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"-" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"+" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"=" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"?" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"\*" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"##" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"#" {
		get_lexer_extra(yyscanner)->modeStack.push(PARAMETER_EXPANSION_MODE);
		emit(EXPANSION_BEGIN, tokenText);
	}

	"^" {
		get_lexer_extra(yyscanner)->modeStack.push(UNLEXED_PARAMETER_EXPANSION_MODE); // TODO: Handle these forms of parameter expansion properly
		emit(EXPANSION_BEGIN, tokenText);
	}

	"," {
		get_lexer_extra(yyscanner)->modeStack.push(UNLEXED_PARAMETER_EXPANSION_MODE); // TODO: Handle these forms of parameter expansion properly
		emit(EXPANSION_BEGIN, tokenText);
	}

	"/" {
		get_lexer_extra(yyscanner)->modeStack.push(UNLEXED_PARAMETER_EXPANSION_MODE); // TODO: Handle these forms of parameter expansion properly
		emit(EXPANSION_BEGIN, tokenText);
	}

	"}" {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BASH_VAR_MODE
		emit(BASH_VAR_END);
	}
}

<PARAMETER_EXPANSION_MODE>{
	"}" {
		get_lexer_extra(yyscanner)->modeStack.pop();
		reconsume_token();
	}
}

<UNLEXED_PARAMETER_EXPANSION_MODE>{
	[^}]+ {
		emit(PARAMETER_EXPANSION_CONTENT, tokenText);
	}

	"}" {
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit PARAMETER_EXPANSION_MODE
		reconsume_token();
	}
}

<BPP_INCLUDE_MODE>{
	{WS} {
		emit(WS, tokenText);
	}
	{SPECIAL_KEYWORD_DYNAMIC} {
		emit(INCLUDE_TYPE, tokenText);
	}
	{SPECIAL_KEYWORD_STATIC} {
		emit(INCLUDE_TYPE, tokenText);
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
		get_lexer_extra(yyscanner)->modeStack.pop(); // Exit BPP_INCLUDE_MODE
		emit(DELIM, tokenText);
	}
}

<*>. {
	emit(CATCHALL, tokenText);
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

extern void initLexer(yyscan_t yyscanner) {
	if (yyget_extra(yyscanner) == nullptr) {
		yyset_extra(new LexerExtra(), yyscanner);
	}
	get_lexer_extra(yyscanner)->modeStack.clear();
	get_lexer_extra(yyscanner)->modeStack.bind(yyscanner);
	get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Initial mode
	get_lexer_extra(yyscanner)->lexerState.reset();
}

extern void destroyLexer(yyscan_t yyscanner) {
	if (yyget_extra(yyscanner) != nullptr) {
		get_lexer_extra(yyscanner)->modeStack.clear();
		get_lexer_extra(yyscanner)->lexerState.reset();
		delete get_lexer_extra(yyscanner);
		yyset_extra(nullptr, yyscanner);
	}
	yylex_destroy(yyscanner);
}

extern void set_utf16_mode(bool enabled, yyscan_t yyscanner) {
	get_lexer_extra(yyscanner)->lexerState.utf16_mode = enabled;
}

yy::parser::symbol_type maybe_get_lvalue_token(yy::parser::symbol_type token, yyscan_t yyscanner) {
	yy::parser::symbol_kind_type type = token.kind();
	auto location = token.location;
	switch (type) {
		case yy::parser::symbol_kind::S_AT:
			// If this AT is the first non-whitespace token of the new command string, it's an lvalue
			if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
				get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false; // Next token cannot be an lvalue
				if (get_lexer_extra(yyscanner)->modeStack.top() == BPP_REFERENCE_MODE) {
					get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = true;
				}
				return yy::parser::make_AT_LVALUE(location);
			}
			break;
		case yy::parser::symbol_kind::S_REF_START:
			// Same as above
			if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
				get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
				get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = true;
				return yy::parser::make_REF_START_LVALUE(location);
			}
			break;
		case yy::parser::symbol_kind::S_IDENTIFIER:
			// Same as above
			if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
				get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
				std::string text = token.value.as<std::string>();
				AST::Token<std::string> text_token(text, location.begin.line, location.begin.column);
				get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = true; // After an identifier lvalue, we may expect an assignment operator
				return yy::parser::make_IDENTIFIER_LVALUE(text_token, location);
			} else if (get_lexer_extra(yyscanner)->lexerState.might_be_lexing_object_instantiation) {
				// This is potentially the name of an instantiated object
				// That means an assignment operator might possibly come next
				get_lexer_extra(yyscanner)->lexerState.might_be_lexing_object_instantiation = false;
				get_lexer_extra(yyscanner)->lexerState.expecting_assignment_operator = true;
				// Carry on from here
			}
			break;
		case yy::parser::symbol_kind::S_KEYWORD_THIS:
			// Same as above
			if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
				get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
				if (get_lexer_extra(yyscanner)->modeStack.top() == BPP_REFERENCE_MODE) {
					get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = true;
				}
				return yy::parser::make_KEYWORD_THIS_LVALUE(location);
			}
			break;
		case yy::parser::symbol_kind::S_KEYWORD_SUPER:
			// Same as above
			if (get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue) {
				get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
				if (get_lexer_extra(yyscanner)->modeStack.top() == BPP_REFERENCE_MODE) {
					get_lexer_extra(yyscanner)->lexerState.currently_lexing_lvalue_reference = true;
				}
				return yy::parser::make_KEYWORD_SUPER_LVALUE(location);
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
		case yy::parser::symbol_kind::S_BASH_CASE_PATTERN_DELIM:
		case yy::parser::symbol_kind::S_BASH_KEYWORD_DO:
		case yy::parser::symbol_kind::S_BASH_KEYWORD_IF:
		case yy::parser::symbol_kind::S_BASH_KEYWORD_ELIF:
		case yy::parser::symbol_kind::S_BASH_KEYWORD_THEN:
		case yy::parser::symbol_kind::S_BASH_KEYWORD_WHILE:
		case yy::parser::symbol_kind::S_BASH_KEYWORD_UNTIL:
			// At the start of a new command string, the next token can be an lvalue
			// And the local keyword can be re-received
			get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = true;
			get_lexer_extra(yyscanner)->lexerState.received_local_keyword = false;
			get_lexer_extra(yyscanner)->modeStack.push(SKIP_AFTER_DELIM_MODE); // Skip whitespace, newlines, and comments after the DELIM
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
		case yy::parser::symbol_kind::S_BASH_KEYWORD_LOCAL:
			// 'local' specifier does not change lvalue considerations
			break;
		case yy::parser::symbol_kind::S_CATCHALL:
			{
				AST::Token<std::string> vl = token.value.as<AST::Token<std::string>>();
				if (vl.getValue() == "!") {
					// '!' operator does not change lvalue considerations
					// Kind of unfortunate that this is caught as a CATCHALL token
					// TODO(@rail5): Separate token type for exit status inversion operator
					break;
				}
			}
		default:
			get_lexer_extra(yyscanner)->lexerState.incoming_token_can_be_lvalue = false;
			break;
	}
	return token; // If we're here, just return the original token unmodified
}
