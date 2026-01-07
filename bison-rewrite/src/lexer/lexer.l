

%{
#include "parser.tab.hpp" // Bison header for token types
#include <cstdint>

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
#define YY_DECL yy::parser::symbol_type yylex()
YY_DECL;

static std::string singlequote_contents; // To hold contents within single quotes

static uint64_t array_index_depth = 0;

%}

%option noyywrap
%option stack

ESCAPED_CHAR    \\.
AT              @

WS              [ \t]+

DELIM           [\n;]

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

IDENTIFIER              [a-zA-Z_][a-zA-Z0-9_]*
INTEGER                 [0-9]+

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

%x BPP_BRACEREFERENCE_MODE

/* Parsing @include statements */
%x BPP_INCLUDE_MODE

/* Handling single-quoted strings as single tokens */
%x SINGLEQUOTE_MODE

/* Handling double-quoted strings as sequences of STRING_TOKEN and interpolations */
%x DOUBLEQUOTE_MODE


/* Handling Bash shell variables */
%x BASH_VAR_MODE

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
		return yy::parser::make_##tokenType(__VA_ARGS__);
	
	/**
	* Helper to get the text of the current token as a std::string.
	*/
	#define tokenText std::string(yytext, yyleng)
%}

{ESCAPED_CHAR}         { emit(ESCAPED_CHAR, tokenText); }
{WS}                   { emit(WS, tokenText); }

{KEYWORD_INCLUDE}      { yy_push_state(BPP_INCLUDE_MODE); emit(KEYWORD_INCLUDE); }
{KEYWORD_INCLUDE_ONCE} { yy_push_state(BPP_INCLUDE_MODE); emit(KEYWORD_INCLUDE_ONCE); }

{KEYWORD_CLASS}        { emit(KEYWORD_CLASS); }
{KEYWORD_PUBLIC}       { emit(KEYWORD_PUBLIC); }
{KEYWORD_PROTECTED}    { emit(KEYWORD_PROTECTED); }
{KEYWORD_PRIVATE}      { emit(KEYWORD_PRIVATE); }
{KEYWORD_NEW}          { emit(KEYWORD_NEW); }
{KEYWORD_METHOD}       { emit(KEYWORD_METHOD); }
{KEYWORD_VIRTUAL}      { emit(KEYWORD_VIRTUAL); }
{KEYWORD_CONSTRUCTOR}  { emit(KEYWORD_CONSTRUCTOR); }
{KEYWORD_DESTRUCTOR}   { emit(KEYWORD_DESTRUCTOR); }
{KEYWORD_DYNAMIC_CAST} { emit(KEYWORD_DYNAMIC_CAST); }

"@{"                   { yy_push_state(BPP_BRACEREFERENCE_MODE); emit(REF_START); }

{AT}                   { yy_push_state(BPP_MODE); emit(AT); }
{DELIM}                { emit(DELIM, tokenText); }
{LBRACE}               { emit(LBRACE); }
{RBRACE}               { emit(RBRACE); }
":"                    { emit(COLON); }
"="                    { emit(EQUALS); }
"*"                    { emit(ASTERISK); }
{IDENTIFIER}           { emit(IDENTIFIER, tokenText); }
{LANGLE}               { emit(LANGLE); }
{RANGLE}               { emit(RANGLE); }

"${"                   { yy_push_state(BASH_VAR_MODE); emit(BASH_VAR_START); }

\${IDENTIFIER}         { emit(BASH_VAR, tokenText); }
\$@                    { emit(BASH_VAR, tokenText); }
\$#                    { emit(BASH_VAR, tokenText); }
\$\*                   { emit(BASH_VAR, tokenText); }
\$-                    { emit(BASH_VAR, tokenText); }
\$\$                   { emit(BASH_VAR, tokenText); }
\$!                    { emit(BASH_VAR, tokenText); }
\$\?                   { emit(BASH_VAR, tokenText); }
\${INTEGER}            { emit(BASH_VAR, tokenText); }

"["                    { emit(LBRACKET); }

"]"                    {
	if (array_index_depth > 0) {
		array_index_depth--;
		yy_pop_state(); // Return to the reference mode that brought us here
		emit(ARRAY_INDEX_END);
		/* TODO: THIS IS WRONG.
		 * array_index_depth > 0 is NOT a sufficient check to see if this is a matching rbracket to an opening lbracket
		 * counter-example: @{object.reference["]"]}
		 *                                     ^
		 */
	} else {
		emit(RBRACKET);
	}
}

['] {
	yy_push_state(SINGLEQUOTE_MODE);
	singlequote_contents.clear();
	singlequote_contents += '\'';
}

["] {
	yy_push_state(DOUBLEQUOTE_MODE);
	emit(QUOTE_BEGIN);
}


<SINGLEQUOTE_MODE>{
	[^'\\]+ {
		/* Consume all characters until closing singlequote */
		singlequote_contents += tokenText;
	}

	{ESCAPED_CHAR} {
		/* If it's an escaped singlequote, it's not the end of the string */
		singlequote_contents += tokenText;
	}

	"'" {
		/* Closing singlequote found */
		yy_pop_state(); // Exit SINGLEQUOTE_MODE
		singlequote_contents += '\'';
		emit(SINGLEQUOTED_STRING, singlequote_contents);
	}
}

<DOUBLEQUOTE_MODE>{
	[^"@\\]+ {
		/* Consume all characters until closing doublequote or interpolation */
		emit(DOUBLEQUOTE_CONTENT, tokenText);
	}

	{ESCAPED_CHAR} {
		/* Handle escaped characters within double quotes */
		emit(DOUBLEQUOTE_CONTENT, tokenText);
	}

	{AT} {
		/* Start of an interpolation */
		yy_push_state(BPP_MODE);
		emit(AT);
	}

	["] {
		/* Closing doublequote found */
		yy_pop_state(); // Exit DOUBLEQUOTE_MODE
		emit(QUOTE_END);
	}
}

<BPP_MODE>{
	{IDENTIFIER} {
		emit(IDENTIFIER, tokenText);
	}

	"\." {
		emit(DOT);
	}

	. {
		yy_pop_state(); // Exit BPP_MODE if we see anything other than IDENTIFIER or DOT coming up next
		yyless(0); // Do not consume the character; let the main lexer mode handle it
	}
}

<BPP_BRACEREFERENCE_MODE>{
	{IDENTIFIER} {
		emit(IDENTIFIER, tokenText);
	}

	"\." {
		emit(DOT);
	}

	"[" {
		yy_push_state(INITIAL);
		array_index_depth++;
		emit(ARRAY_INDEX_START);
	}

	"}" {
		yy_pop_state(); // Exit BPP_BRACEREFERENCE_MODE
		emit(REF_END);
	}
}

<BASH_VAR_MODE>{
	{IDENTIFIER} {
		emit(IDENTIFIER, tokenText);
	}
	
	"[" {
		yy_push_state(INITIAL);
		array_index_depth++;
		emit(ARRAY_INDEX_START);
	}

	"}" {
		yy_pop_state(); // Exit BASH_VAR_MODE
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
		yy_pop_state(); // Exit BPP_INCLUDE_MODE
		emit(DELIM, tokenText);
	}
}

<*>. {
	emit(ERROR, tokenText);
}

%%
