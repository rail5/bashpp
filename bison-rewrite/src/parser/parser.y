%language "c++"
%require "3.2"

%code requires {
#include <memory>
}

%{
#include <iostream>
#include <memory>
void yyerror(const char *s);
%}


%define parse.error verbose

%define api.token.constructor
%define api.value.type variant
%code {
	#include "parser.tab.hpp"

	yy::parser::symbol_type yylex();
}

%token <std::string> ESCAPED_CHAR WS DELIM

%token <std::string> SINGLEQUOTED_STRING

%token QUOTE_BEGIN QUOTE_END
%token <std::string> DOUBLEQUOTE_CONTENT

%token AT
%token LBRACE RBRACE LANGLE RANGLE
%token COLON EQUALS ASTERISK DOT

%token KEYWORD_INCLUDE KEYWORD_INCLUDE_ONCE KEYWORD_AS KEYWORD_DYNAMIC_CAST
%token <std::string> INCLUDE_TYPE INCLUDE_PATH

%token KEYWORD_CLASS KEYWORD_NEW KEYWORD_VIRTUAL KEYWORD_METHOD KEYWORD_CONSTRUCTOR KEYWORD_DESTRUCTOR

%token <std::string> IDENTIFIER

%token KEYWORD_PUBLIC KEYWORD_PRIVATE KEYWORD_PROTECTED

%token ARRAY_INDEX_START ARRAY_INDEX_END LBRACKET RBRACKET
%token REF_START REF_END
%token <std::string> BASH_VAR
%token BASH_VAR_START BASH_VAR_END

/* Handling unrecognized tokens */
%token <std::string> ERROR


/* Nonterminal types */
%type <int> include_keyword access_modifier access_modifier_keyword
%type <std::string> maybe_include_type maybe_as_clause maybe_parent_class maybe_default_value
%type <std::string> valid_rvalue
%type <std::string> doublequoted_string quote_contents
%type <std::string> object_reference maybe_descend_object_hierarchy maybe_array_index
%type <std::string> bash_variable
%type <std::string> dynamic_cast cast_target

/**
 * NOTE: A shift/reduce conflict is EXPECTED between 'object_instantiation' and
 * 'object_reference' on the WS token.
 *
 * This is because, with only 1 token of lookahead, the parser cannot determine
 * whether the WS is between the class name and object name in an object
 * instantiation, or if instead it's AFTER a complete object reference
 * (e.g., @objectReference unrelatedToken, which would make 'unrelatedToken' an
 * argument to the implicit .toPrimitive call).
 *
 * The parser doesn't know whether to SHIFT (i.e., treat the WS as part of the
 * current statement and keep going), or REDUCE (i.e., take the opportunity to
 * say "I've now scanned enough tokens to know which statement type this is").
 *
 * However, this conflict is benign, and the default behavior (shifting the WS
 * and treating it as part of an object instantiation) is the desired one.
 *
 * In other words, there is an INHERENT AMBIGUITY in the Bash++ grammar that
 * cannot be resolved with an LALR(1) parser.
 *
 *     @ID ID
 * Is obviously an object instantiation, right?
 *
 * However: Bash++ grammar also says explicitly that:
 *     A reference to a non-primitive object in a place where a primitive is
 *     expected is implicitly a call to the .toPrimitive method of that object.
 *
 * So, the same statement could reasonably be interpreted as @ID.toPrimitive
 * with the second 'ID' as an argument
 *
 * Our resolution to this is to say simply that no arguments can be passed to
 * .toPrimitive IFF it is only IMPLIED by referencing the object directly in a
 * primitive context.
 *
 * If the user actually wants to call .toPrimitive with arguments, they must do
 * so explicitly, as in:
 *    @object.toPrimitive argument1 argument2
 *
 * The default behavior of Bison in the presence of this shift/reduce conflict
 * is to favor shifting, which is exactly what we want.
 */

%%

program: statements;

statements:
	/* empty */
	| statements statement
	;

statement:
	DELIM
	| WS
	| include_statement
	| class_definition
	| datamember_declaration
	| method_definition
	| constructor_definition
	| destructor_definition
	| object_instantiation
	| pointer_declaration
	| new_statement
	| object_reference
	| block
	| BASH_VAR
	| bash_variable
	| dynamic_cast
	;

block:
	LBRACE statements RBRACE
	;

valid_rvalue:
	/* empty */ { $$ = ""; }
	| IDENTIFIER { $$ = $1; }
	| SINGLEQUOTED_STRING { $$ = $1; }
	| doublequoted_string { $$ = $1; }
	| new_statement { $$ = ""; }
	| object_reference { $$ = $1; }
	| BASH_VAR { $$ = $1; }
	| bash_variable { $$ = $1; }
	| dynamic_cast {$$ = $1; }
	;

maybe_whitespace:
	/* empty */
	| WS
	;

whitespace_or_delimiter:
	WS
	| DELIM
	;

include_statement:
	include_keyword maybe_include_type INCLUDE_PATH maybe_as_clause DELIM {
		std::string includeKeyword = $1 == yy::parser::token::KEYWORD_INCLUDE ? "include" : "include_once";
		std::string includeType = $2.empty() ? "default" : $2;
		std::string includePath = $3;
		std::string asPath = $4.empty() ? "" : $4;

		std::cout << "Parsed include statement: "
		          << "Keyword='" << includeKeyword << "', "
		          << "Type='" << includeType << "', "
		          << "Path='" << includePath << "'";
		if (!asPath.empty()) {
			std::cout << ", As='" << asPath << "'";
		}
		std::cout << std::endl;
	}
	;

include_keyword:
	KEYWORD_INCLUDE { $$ = yy::parser::token::KEYWORD_INCLUDE; }
	| KEYWORD_INCLUDE_ONCE { $$ = yy::parser::token::KEYWORD_INCLUDE_ONCE; }
	;

maybe_include_type:
	WS { $$ = ""; }
	| WS INCLUDE_TYPE WS { $$ = $2; }
	;

maybe_as_clause:
	/* empty */ { $$ = ""; }
	| WS KEYWORD_AS WS INCLUDE_PATH { $$ = $4; }
	;

object_instantiation:
	AT IDENTIFIER WS IDENTIFIER DELIM {
		std::string className = $2;
		std::string objectName = $4;

		std::cout << "Parsed object instantiation: Class='" << className << "', Object='" << objectName << "'" << std::endl;
	}
	;

pointer_declaration:
	AT IDENTIFIER ASTERISK WS IDENTIFIER maybe_default_value DELIM {
		std::string typeName = $2;
		std::string pointerName = $5;
		std::string defaultValue = $6;

		std::cout << "Parsed pointer declaration: Type='" << typeName << "', Pointer='" << pointerName << "'";
		if (!defaultValue.empty()) {
			std::cout << ", Default='" << defaultValue << "'";
		}
		std::cout << std::endl;
	}

new_statement:
	KEYWORD_NEW WS IDENTIFIER {
		std::string className = $3;

		std::cout << "Parsed new statement: Class='" << className << "'" << std::endl;
	}

class_definition:
	KEYWORD_CLASS WS IDENTIFIER maybe_parent_class block {
		std::string className = $3;
		std::string parentClass = $4;

		std::cout << "Parsed class definition: Name='" << className << "'";
		if (!parentClass.empty()) {
			std::cout << ", Parent='" << parentClass << "'";
		}
		std::cout << std::endl;
	}

maybe_parent_class:
	whitespace_or_delimiter { $$ = ""; }
	| WS COLON WS IDENTIFIER whitespace_or_delimiter { $$ = $4; }
	;

datamember_declaration:
	access_modifier IDENTIFIER maybe_default_value DELIM {
		std::string accessMod;
		switch ($1) {
			case yy::parser::token::KEYWORD_PUBLIC:
				accessMod = "public";
				break;
			case yy::parser::token::KEYWORD_PRIVATE:
				accessMod = "private";
				break;
			case yy::parser::token::KEYWORD_PROTECTED:
				accessMod = "protected";
				break;
			default:
				accessMod = "unknown";
				break;
		}
		std::string memberName = $2;

		std::string defaultValue = $3;

		std::cout << "Parsed data member declaration: Name='" << memberName << "', Access='" << accessMod << "'";
		if (!defaultValue.empty()) {
			std::cout << ", Default='" << defaultValue << "'";
		}
		std::cout << std::endl;
	}
	| access_modifier object_instantiation {
		std::string accessMod;
		switch ($1) {
			case yy::parser::token::KEYWORD_PUBLIC:
				accessMod = "public";
				break;
			case yy::parser::token::KEYWORD_PRIVATE:
				accessMod = "private";
				break;
			case yy::parser::token::KEYWORD_PROTECTED:
				accessMod = "protected";
				break;
			default:
				accessMod = "unknown";
				break;
		}

		std::cout << "Parsed object instantiation with access modifier: Access='" << accessMod << "'" << std::endl;
	}
	| access_modifier pointer_declaration {
		std::string accessMod;
		switch ($1) {
			case yy::parser::token::KEYWORD_PUBLIC:
				accessMod = "public";
				break;
			case yy::parser::token::KEYWORD_PRIVATE:
				accessMod = "private";
				break;
			case yy::parser::token::KEYWORD_PROTECTED:
				accessMod = "protected";
				break;
			default:
				accessMod = "unknown";
				break;
		}

		std::cout << "Parsed pointer declaration with access modifier: Access='" << accessMod << "'" << std::endl;
	}
	;

access_modifier:
	access_modifier_keyword WS {
		$$ = $1;
	}
	;

access_modifier_keyword:
	KEYWORD_PUBLIC { $$ = yy::parser::token::KEYWORD_PUBLIC; }
	| KEYWORD_PRIVATE { $$ = yy::parser::token::KEYWORD_PRIVATE; }
	| KEYWORD_PROTECTED { $$ = yy::parser::token::KEYWORD_PROTECTED; }
	;

maybe_default_value:
	maybe_whitespace { $$ = ""; }
	| EQUALS valid_rvalue { $$ = $2; }
	;

method_definition:
	access_modifier KEYWORD_METHOD WS IDENTIFIER WS maybe_parameter_list block {
		std::string methodName = $4;

		std::cout << "Parsed method definition: Name='" << methodName << "'" << std::endl;
	}
	| KEYWORD_VIRTUAL WS access_modifier KEYWORD_METHOD WS IDENTIFIER WS maybe_parameter_list block {
		std::string methodName = $6;

		std::cout << "Parsed virtual method definition: Name='" << methodName << "'" << std::endl;
	}
	;

maybe_parameter_list:
	/* empty */
	| maybe_parameter_list parameter
	;

parameter:
	IDENTIFIER WS { std::cout << "Primitive parameter: " << $1 << std::endl; }
	| AT IDENTIFIER ASTERISK WS IDENTIFIER WS { std::cout << "Pointer parameter: Type=" << $2 << ", Name=" << $5 << std::endl; }
	| AT IDENTIFIER WS IDENTIFIER WS /* Actually invalid, but error handling should come later when traversing the AST */
		/* Invalid because methods cannot take non-primitive parameters */
	;

constructor_definition:
	KEYWORD_CONSTRUCTOR WS block {
		std::cout << "Parsed constructor definition" << std::endl;
	}
	;

destructor_definition:
	KEYWORD_DESTRUCTOR WS block {
		std::cout << "Parsed destructor definition" << std::endl;
	}
	;

doublequoted_string:
	QUOTE_BEGIN quote_contents QUOTE_END {
		std::cout << "Parsed double-quoted string: " << $2 << std::endl;
		$$ = "\"" + $2 + "\"";
	}
	;

quote_contents:
	/* empty */ { $$ = ""; }
	| quote_contents DOUBLEQUOTE_CONTENT { $$ = $1 + $2; }
	| quote_contents AT IDENTIFIER { $$ = $1 + "@" + $3; }
	;

object_reference:
	AT IDENTIFIER maybe_descend_object_hierarchy {
		std::string objectName = $2;
		std::string hierarchy = $3;

		std::cout << "Parsed object reference: Object='" << objectName << "'";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		std::cout << std::endl;

		$$ = "@" + objectName + hierarchy;
	}
	| REF_START IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string objectName = $2;
		std::string hierarchy = $3;
		std::string arrayIndex = $4;

		std::cout << "Parsed reference object reference: Object='" << objectName << "'";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "@" + objectName + hierarchy + arrayIndex;
	}
	;

maybe_descend_object_hierarchy:
	/* empty */ { $$ = ""; }
	| DOT IDENTIFIER maybe_descend_object_hierarchy {
		$$ = "." + $2 + $3;
	}
	;

maybe_array_index:
	/* empty */ { $$ = ""; }
	| ARRAY_INDEX_START valid_rvalue ARRAY_INDEX_END {
		$$ = "[" + $2 + "]";
	}
	;

bash_variable:
	BASH_VAR_START IDENTIFIER maybe_array_index BASH_VAR_END {
		std::string varName = $2;
		std::string arrayIndex = $3;

		std::cout << "Parsed Bash variable: Name='" << varName << "'";
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "${" + varName + arrayIndex + "}";
	}

dynamic_cast:
	KEYWORD_DYNAMIC_CAST LANGLE cast_target RANGLE WS valid_rvalue {
		std::string targetType = $3;
		std::string expression = $6;

		std::cout << "Parsed dynamic_cast: TargetType='" << targetType << "', Expression='" << expression << "'" << std::endl;

		$$ = "dynamic_cast<" + targetType + ">(" + expression + ")";
	}
	;

cast_target:
	IDENTIFIER {
		std::string targetType = $1;
		std::cout << "Parsed dynamic_cast target type: " << targetType << std::endl;
		$$ = targetType;
	}
	| BASH_VAR {
		std::string targetType = $1;
		std::cout << "Parsed dynamic_cast target type (Bash variable): " << targetType << std::endl;
		$$ = targetType;
	}
	| bash_variable {
		std::string targetType = $1;
		std::cout << "Parsed dynamic_cast target type (Bash variable): " << targetType << std::endl;
		$$ = targetType;
	}
	| object_reference {
		std::string targetType = $1;
		std::cout << "Parsed dynamic_cast target type (object reference): " << targetType << std::endl;
		$$ = targetType;
	}
	;

%%

namespace yy {
void parser::error(const std::string& m) {
	std::cerr << "Parse error: " << m << std::endl;
}
} // namespace yy
