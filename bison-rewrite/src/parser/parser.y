%language "c++"
%require "3.2"

%code requires {
#include <memory>
#include <cassert>
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

	extern void set_incoming_token_can_be_lvalue(bool canBeLvalue);
	extern void set_bash_case_input_received(bool received);
	extern void set_bash_for_or_select_variable_received(bool received);
	extern void set_parsed_assignment_operator(bool parsed);
}

%token <std::string> ESCAPED_CHAR WS DELIM
%token DOUBLEAMPERSAND DOUBLEPIPE PIPE

%token <std::string> SINGLEQUOTED_STRING

%token QUOTE_BEGIN QUOTE_END
%token <std::string> STRING_CONTENT

%token AT AT_LVALUE
%token KEYWORD_THIS KEYWORD_THIS_LVALUE KEYWORD_SUPER KEYWORD_SUPER_LVALUE
%token LBRACE RBRACE LANGLE RANGLE
%token COLON PLUS_EQUALS EQUALS ASTERISK DEREFERENCE_OPERATOR AMPERSAND DOT
%token EMPTY_ASSIGNMENT

%token KEYWORD_INCLUDE KEYWORD_INCLUDE_ONCE KEYWORD_AS KEYWORD_DYNAMIC_CAST
%token <std::string> INCLUDE_TYPE INCLUDE_PATH

%token SUPERSHELL_START SUPERSHELL_END SUBSHELL_START SUBSHELL_END SUBSHELL_SUBSTITUTION_START SUBSHELL_SUBSTITUTION_END
%token <int> DEPRECATED_SUBSHELL_START DEPRECATED_SUBSHELL_END
%token LPAREN RPAREN

%token KEYWORD_CLASS KEYWORD_VIRTUAL KEYWORD_METHOD KEYWORD_CONSTRUCTOR KEYWORD_DESTRUCTOR
%token KEYWORD_NEW KEYWORD_DELETE KEYWORD_NULLPTR

%token <std::string> IDENTIFIER IDENTIFIER_LVALUE

%token KEYWORD_PUBLIC KEYWORD_PRIVATE KEYWORD_PROTECTED
%token KEYWORD_TYPEOF

%token ARRAY_INDEX_START ARRAY_INDEX_END LBRACKET RBRACKET
%token REF_START REF_START_LVALUE REF_END
%token <std::string> BASH_VAR
%token BASH_VAR_START BASH_VAR_END
%token HASH
%token HEREDOC_START HERESTRING_START
%token <std::string> HEREDOC_DELIMITER HEREDOC_END

%token BASH_KEYWORD_CASE BASH_KEYWORD_IN BASH_CASE_PATTERN_DELIM BASH_CASE_PATTERN_TERMINATOR BASH_KEYWORD_ESAC
%token <std::string> BASH_CASE_BODY_BEGIN
%token BASH_KEYWORD_SELECT BASH_KEYWORD_FOR BASH_KEYWORD_DO BASH_KEYWORD_DONE
%token ARITH_FOR_CONDITION_START ARITH_FOR_CONDITION_END
%token INCREMENT_OPERATOR DECREMENT_OPERATOR
%token <std::string> INTEGER COMPARISON_OPERATOR

/* Handling unrecognized tokens */
%token <std::string> ERROR


/* Nonterminal types */
%type <int> include_keyword access_modifier access_modifier_keyword
%type <std::string> maybe_include_type maybe_as_clause maybe_parent_class maybe_default_value
%type <std::string> valid_rvalue value_assignment assignment_operator
%type <std::string> doublequoted_string quote_contents
%type <std::string> object_reference object_reference_lvalue maybe_descend_object_hierarchy maybe_array_index
%type <std::string> self_reference self_reference_lvalue
%type <std::string> object_assignment shell_variable_assignment
%type <std::string> bash_variable
%type <std::string> dynamic_cast cast_target
%type <std::string> object_address pointer_dereference pointer_dereference_rvalue pointer_dereference_lvalue
%type <std::string> supershell subshell subshell_raw subshell_substitution dollar_subshell deprecated_subshell
%type <std::string> string_interpolation
%type <std::string> maybe_hash
%type <std::string> typeof_expression
%type <std::string> heredoc heredoc_content herestring
%type <std::string> array_index
%type <std::string> bash_case_body bash_case_header bash_case_input bash_case_pattern bash_case_statement bash_case_pattern_header
%type <std::string> bash_select_statement bash_for_statement
%type <std::string> bash_for_or_select_header bash_for_or_select_input bash_for_or_select_variable bash_for_or_select_maybe_in_something
%type <std::string> bash_arithmetic_for_statement arithmetic_for_condition arith_statement increment_decrement_expression arith_operator
%type <std::string> arith_condition_term comparison_expression comparison_operator
%type <std::string> redirection redirection_preposition redirection_source redirection_operator named_fd maybe_namedfd_array_index

/**
 * NOTE: A shift/reduce conflict is EXPECTED between 'object_instantiation' and
 * 'object_reference_lvalue' on the WS token.
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
	| delete_statement
	| object_reference
	| object_reference_lvalue
	| self_reference
	| self_reference_lvalue
	| object_address
	| pointer_dereference
	| shell_variable_assignment
	| object_assignment
	| block
	| bash_variable
	| dynamic_cast
	| supershell
	| subshell
	| typeof_expression
	| bash_case_statement
	| bash_select_statement
	| bash_for_statement
	| bash_arithmetic_for_statement
	| redirection
	;

/**
 * NOTE: A shift/reduce conflict is expected on AMPERSAND
 * Example redirection: >&@object.reference
 * Ambiguity: In this case, is it > &@object.address? Or is it >& @object.reference?
 * Our resolution to this is to prefer the parser's default behavior of shifting
 *   which would resolve to >& @object.reference
 * If you want to redirect to an object address, whitespace is required before the address-of operator, as in:
 *   > &@object.address
 */
redirection:
	redirection_preposition maybe_whitespace valid_rvalue {
		std::string redirPreposition = $1;
		std::string rvalue = $3;

		std::cout << "Parsed redirection: Preposition='" << redirPreposition << "', RValue='" << rvalue << "'" << std::endl;

		set_incoming_token_can_be_lvalue(true); // Lvalues can follow redirections

		$$ = redirPreposition + rvalue;
	}
	| heredoc {
		std::string heredocContent = $1;

		std::cout << "Parsed heredoc redirection with content: " << heredocContent << std::endl;

		$$ = $1;
	}
	| INTEGER heredoc {
		std::string fd = $1;
		std::string heredocContent = $2;

		std::cout << "Parsed heredoc redirection for FD " << fd << " with content: " << heredocContent << std::endl;

		$$ = $1 + $2;
	}
	| named_fd heredoc {
		std::string fd = $1;
		std::string heredocContent = $2;

		std::cout << "Parsed heredoc redirection for FD " << fd << " with content: " << heredocContent << std::endl;

		$$ = $1 + $2;
	}
	;

redirection_preposition:
	redirection_source redirection_operator {
		std::string source = $1;
		std::string redirOp = $2;

		std::cout << "Parsed redirection preposition: Source='" << source << "', Operator='" << redirOp << "'" << std::endl;
	}
	| AMPERSAND RANGLE { $$ = "&>"; } // &>
	| AMPERSAND RANGLE RANGLE { $$ = "&>>"; } // &>>
	;

redirection_source:
	/* empty */ { $$ = ""; }
	| INTEGER { $$ = $1; }
	| named_fd { $$ = $1; }
	;

redirection_operator:
	LANGLE { $$ = "<"; }
	| LANGLE RANGLE { $$ = "<>"; }
	| LANGLE AMPERSAND { $$ = "<&"; }
	| RANGLE { $$ = ">"; }
	| RANGLE RANGLE { $$ = ">>"; }
	| RANGLE AMPERSAND { $$ = ">&"; }
	| RANGLE PIPE { $$ = ">|"; }
	| herestring { $$ = $1; }
	;

named_fd:
	LBRACE IDENTIFIER maybe_namedfd_array_index RBRACE { $$ = "{" + $2 + $3 + "}"; }
	;

maybe_namedfd_array_index:
	/* empty */ { $$ = ""; }
	| LBRACKET array_index RBRACKET { $$ = "[" + $2 + "]"; } // Since LBRACKET/RBRACKET don't get matched as ARRAY_INDEX_START/_END in normal lexer mode
	;

block:
	LBRACE statements RBRACE
	;

valid_rvalue:
	EMPTY_ASSIGNMENT { $$ = ""; }
	| IDENTIFIER { $$ = $1; }
	| INTEGER { $$ = $1; }
	| SINGLEQUOTED_STRING { $$ = $1; }
	| KEYWORD_NULLPTR { $$ = "0"; }
	| doublequoted_string { $$ = $1; }
	| new_statement { $$ = ""; }
	| object_reference { $$ = $1; }
	| self_reference { $$ = $1; }
	| object_address { $$ = $1; }
	| pointer_dereference_rvalue { $$ = $1; }
	| bash_variable { $$ = $1; }
	| dynamic_cast {$$ = $1; }
	| supershell { $$ = $1; }
	| subshell_substitution { $$ = $1; }
	| subshell_raw { $$ = $1; } // Not actually subshells in the case of rvalues, but array values, as in arr+=("string"). Kind of a hack.
	| typeof_expression { $$ = $1; }
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
	AT_LVALUE IDENTIFIER WS IDENTIFIER DELIM {
		std::string className = $2;
		std::string objectName = $4;

		std::cout << "Parsed object instantiation: Class='" << className << "', Object='" << objectName << "'" << std::endl;
	}
	| AT_LVALUE IDENTIFIER WS error {
		/* TODO(@rail5): Tie this into the compiler's actual error reporting function to display line/col etc */
		std::cerr << "Syntax error in object instantiation: An object name is required after the class name."
		<< std::endl
		<< "If your intention was to call .toPrimitive with arguments, please do so explicitly, as in:"
		<< std::endl
		<< "	@" << $2 << ".toPrimitive <arg1> <arg2> ..."
		<< std::endl;
		yyerrok;
	}
	;

pointer_declaration:
	AT_LVALUE IDENTIFIER ASTERISK WS IDENTIFIER maybe_default_value DELIM {
		std::string typeName = $2;
		std::string pointerName = $5;
		std::string defaultValue = $6;

		std::cout << "Parsed pointer declaration: Type='" << typeName << "', Pointer='" << pointerName << "'";
		if (!defaultValue.empty()) {
			std::cout << ", Default='" << defaultValue << "'";
		}
		std::cout << std::endl;
	}
	;

new_statement:
	KEYWORD_NEW WS IDENTIFIER {
		std::string className = $3;

		std::cout << "Parsed new statement: Class='" << className << "'" << std::endl;
	}
	;

delete_statement:
	KEYWORD_DELETE WS object_reference {
		std::string objectRef = $3;

		std::cout << "Parsed delete statement: ObjectReference='" << objectRef << "'" << std::endl;
	}
	|
	KEYWORD_DELETE WS self_reference {
		std::string selfRef = $3;

		std::cout << "Parsed delete statement: SelfReference='" << selfRef << "'" << std::endl;
	}
	;

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
	;

maybe_parent_class:
	whitespace_or_delimiter { $$ = ""; }
	| WS COLON WS IDENTIFIER whitespace_or_delimiter { $$ = $4; }
	;

datamember_declaration:
	access_modifier IDENTIFIER_LVALUE maybe_default_value DELIM {
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
	| value_assignment { $$ = $1; }
	;

value_assignment:
	assignment_operator valid_rvalue {
		std::string assignOp = $1;
		std::string rvalue = $2;

		std::cout << "Parsed value assignment: Operator='" << assignOp << "', RValue='" << rvalue << "'" << std::endl;

		$$ = $2;
	}
	;

assignment_operator:
	EQUALS {
		set_parsed_assignment_operator(true);
		$$ = "=";
	}
	| PLUS_EQUALS {
		set_parsed_assignment_operator(true);
		$$ = "+=";
	}
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
	| quote_contents STRING_CONTENT { $$ = $1 + $2; }
	| quote_contents string_interpolation { $$ = $1 + $2; }
	;

string_interpolation:
	object_reference { $$ = $1; }
	| self_reference { $$ = $1; }
	| object_address { $$ = $1; }
	| pointer_dereference { $$ = $1; }
	| supershell { $$ = $1; }
	| subshell_substitution { $$ = $1; }
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
	| REF_START maybe_hash IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string objectName = $3;
		std::string hierarchy = $4;
		std::string arrayIndex = $5;

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

object_reference_lvalue:
	AT_LVALUE IDENTIFIER maybe_descend_object_hierarchy {
		std::string objectName = $2;
		std::string hierarchy = $3;

		std::cout << "Parsed lvalue object reference: Object='" << objectName << "'";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		std::cout << std::endl;

		$$ = "@" + objectName + hierarchy;
	}
	| REF_START_LVALUE maybe_hash IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string objectName = $3;
		std::string hierarchy = $4;
		std::string arrayIndex = $5;

		std::cout << "Parsed lvalue reference object reference: Object='" << objectName << "'";
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

self_reference:
	KEYWORD_THIS maybe_descend_object_hierarchy {
		std::string hierarchy = $2;

		std::cout << "Parsed self reference";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		std::cout << std::endl;

		$$ = "@this" + hierarchy;
	}
	| REF_START maybe_hash KEYWORD_THIS maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string hierarchy = $4;
		std::string arrayIndex = $5;

		std::cout << "Parsed reference self reference";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "@this" + hierarchy + arrayIndex;
	}
	| KEYWORD_SUPER maybe_descend_object_hierarchy {
		std::string hierarchy = $2;

		std::cout << "Parsed self reference to super";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		std::cout << std::endl;

		$$ = "@super" + hierarchy;
	}
	| REF_START maybe_hash KEYWORD_SUPER maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string hierarchy = $4;
		std::string arrayIndex = $5;

		std::cout << "Parsed reference self reference to super";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "@super" + hierarchy + arrayIndex;
	}
	;

self_reference_lvalue:
	KEYWORD_THIS_LVALUE maybe_descend_object_hierarchy {
		std::string hierarchy = $2;

		std::cout << "Parsed lvalue self reference";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		std::cout << std::endl;

		$$ = "@this" + hierarchy;
	}
	| REF_START_LVALUE maybe_hash KEYWORD_THIS maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string hierarchy = $4;
		std::string arrayIndex = $5;

		std::cout << "Parsed lvalue reference self reference";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "@this" + hierarchy + arrayIndex;
	}
	| KEYWORD_SUPER_LVALUE maybe_descend_object_hierarchy {
		std::string hierarchy = $2;

		std::cout << "Parsed lvalue self reference to super";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		std::cout << std::endl;

		$$ = "@super" + hierarchy;
	}
	| REF_START_LVALUE maybe_hash KEYWORD_SUPER maybe_descend_object_hierarchy maybe_array_index REF_END {
		std::string hierarchy = $4;
		std::string arrayIndex = $5;

		std::cout << "Parsed lvalue reference self reference to super";
		if (!hierarchy.empty()) {
			std::cout << ", Hierarchy='" << hierarchy << "'";
		}
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "@super" + hierarchy + arrayIndex;
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
	| ARRAY_INDEX_START array_index ARRAY_INDEX_END {
		$$ = "[" + $2 + "]";
	}
	;

array_index:
	valid_rvalue { $$ = $1; }
	| AT { $$ = "@"; } // '@' is a valid array index, as in ${array[@]}
	;

maybe_hash:
	/* empty */ { $$ = ""; }
	| HASH { $$ = "#"; }
	;

bash_variable:
	BASH_VAR_START maybe_hash IDENTIFIER maybe_array_index BASH_VAR_END {
		std::string varName = $3;
		std::string arrayIndex = $4;

		std::cout << "Parsed Bash variable: Name='" << varName << "'";
		if (!arrayIndex.empty()) {
			std::cout << ", ArrayIndex='" << arrayIndex << "'";
		}
		std::cout << std::endl;

		$$ = "${" + varName + arrayIndex + "}";
	}
	| BASH_VAR { $$ = $1; }
	;

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

object_assignment:
	object_reference_lvalue value_assignment {
		std::string objectRef = $1;
		std::string rvalue = $2;

		std::cout << "Parsed object assignment: ObjectReference='" << objectRef << "', RValue='" << rvalue << "'" << std::endl;

		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		$$ = $1 + "=" + $2;
	}
	| self_reference_lvalue value_assignment {
		std::string selfRef = $1;
		std::string rvalue = $2;

		std::cout << "Parsed self assignment: SelfReference='" << selfRef << "', RValue='" << rvalue << "'" << std::endl;

		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		$$ = $1 + "=" + $2;
	}
	| pointer_dereference_lvalue value_assignment {
		std::string pointerDeref = $1;
		std::string rvalue = $2;

		std::cout << "Parsed pointer dereference assignment: PointerDereference='" << pointerDeref << "', RValue='" << rvalue << "'" << std::endl;

		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		$$ = $1 + "=" + $2;
	}
	;

shell_variable_assignment:
	IDENTIFIER_LVALUE value_assignment {
		std::string varName = $1;
		std::string rvalue = $2;

		std::cout << "Parsed shell variable assignment: Variable='" << varName << "', RValue='" << rvalue << "'" << std::endl;

		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		$$ = varName + "=" + rvalue;
	}
	;

object_address:
	AMPERSAND object_reference {
		std::string objectRef = $2;

		std::cout << "Parsed object address-of: ObjectReference='" << objectRef << "'" << std::endl;

		$$ = "&" + objectRef;
	}
	| AMPERSAND self_reference {
		std::string selfRef = $2;

		std::cout << "Parsed self address-of: SelfReference='" << selfRef << "'" << std::endl;

		$$ = "&" + selfRef;
	}
	;

pointer_dereference:
	pointer_dereference_rvalue { $$ = $1; }
	| pointer_dereference_lvalue { $$ = $1; }
	;

pointer_dereference_rvalue:
	DEREFERENCE_OPERATOR object_reference {
		std::string objectRef = $2;

		std::cout << "Parsed pointer dereference: ObjectReference='" << objectRef << "'" << std::endl;

		$$ = "*" + objectRef;
	}
	| DEREFERENCE_OPERATOR self_reference {
		std::string selfRef = $2;

		std::cout << "Parsed self pointer dereference: SelfReference='" << selfRef << "'" << std::endl;

		$$ = "*" + selfRef;
	}
	;

pointer_dereference_lvalue:
	DEREFERENCE_OPERATOR object_reference_lvalue {
		std::string objectRef = $2;

		std::cout << "Parsed lvalue pointer dereference: ObjectReference='" << objectRef << "'" << std::endl;

		$$ = "*" + objectRef;
	}
	| DEREFERENCE_OPERATOR self_reference_lvalue {
		std::string selfRef = $2;

		std::cout << "Parsed lvalue self pointer dereference: SelfReference='" << selfRef << "'" << std::endl;

		$$ = "*" + selfRef;
	}
	;

typeof_expression:
	KEYWORD_TYPEOF WS valid_rvalue {
		std::string expression = $3;

		std::cout << "Parsed typeof expression: Expression='" << expression << "'" << std::endl;

		$$ = "typeof(" + expression + ")";
	}

supershell:
	SUPERSHELL_START statements SUPERSHELL_END {
		std::cout << "Parsed supershell block" << std::endl;
		$$ = "@(supershell)";
	}
	;

subshell:
	subshell_raw { $$ = $1; }
	| subshell_substitution { $$ = $1; }
	;

subshell_raw:
	SUBSHELL_START statements SUBSHELL_END {
		std::cout << "Parsed subshell block" << std::endl;
		$$ = "(subshell)";
	}
	;

subshell_substitution:
	dollar_subshell { $$ = $1; }
	| deprecated_subshell { $$ = $1; }
	;

dollar_subshell:
	SUBSHELL_SUBSTITUTION_START statements SUBSHELL_SUBSTITUTION_END {
		std::cout << "Parsed subshell substitution block" << std::endl;
		$$ = "$(subshell_substitution)";
	}
	;

deprecated_subshell:
	DEPRECATED_SUBSHELL_START statements DEPRECATED_SUBSHELL_END {
		// NOTE: The nesting depth is stored as the semantic value of the DEPRECATED_SUBSHELL_START token
		assert($1 == $3 && "Mismatched deprecated subshell nesting depths!");

		std::cout << "Parsed DEPRECATED subshell block [depth = " << $1 << "]" << std::endl;
		$$ = "`deprecated_subshell`";
	}
	;

heredoc:
	HEREDOC_START HEREDOC_DELIMITER DELIM heredoc_content HEREDOC_END {
		std::string delimiter = $2;
		std::string content = $4;

		assert($2 == $5 && "Mismatched heredoc delimiters!");

		std::cout << "Parsed heredoc: Delimiter='" << delimiter << "', Content='" << content << "'" << std::endl;

		$$ = "<<"+ delimiter + "\n" + content + "\n" + delimiter;
	}
	;

heredoc_content:
	/* empty */ { $$ = ""; }
	| heredoc_content STRING_CONTENT { $$ = $1 + $2; }
	| heredoc_content string_interpolation { $$ = $1 + $2; }
	| heredoc_content heredoc { $$ = $1 + $2; }
	;

herestring:
	HERESTRING_START maybe_whitespace valid_rvalue {
		std::string rvalue = $3;

		std::cout << "Parsed herestring: RValue='" << rvalue << "'" << std::endl;

		$$ = "<<< " + rvalue;
	}
	;

bash_case_statement:
	BASH_KEYWORD_CASE WS bash_case_header bash_case_body BASH_KEYWORD_ESAC {
		std::string caseHeader = $3;
		std::string caseBody = $4;

		std::cout << "Parsed bash case statement" << std::endl;

		$$ = "case " + caseHeader + " in\n" + caseBody + "\nesac";
	}
	;

bash_case_header:
	bash_case_input WS BASH_KEYWORD_IN BASH_CASE_BODY_BEGIN {
		std::string caseInput = $1;
		std::cout << "Parsed bash case header: Input='" << caseInput << "'" << std::endl;
		$$ = caseInput;
	}
	;

bash_case_input:
	valid_rvalue {
		set_bash_case_input_received(true);
		$$ = $1;
	}
	;

bash_case_body:
	/* empty */ { $$ = ""; }
	| bash_case_body bash_case_pattern { $$ = $1 + $2; }
	;

bash_case_pattern:
	bash_case_pattern_header BASH_CASE_PATTERN_DELIM statements BASH_CASE_PATTERN_TERMINATOR {
		std::string pattern = $1;

		std::cout << "Parsed bash case pattern: Pattern='" << pattern << "'" << std::endl;

		$$ = pattern + "... statements ...;;";
	}
	;

bash_case_pattern_header:
	/* empty */ { $$ = ""; }
	| bash_case_pattern_header STRING_CONTENT { $$ = $1 + $2; }
	| bash_case_pattern_header string_interpolation { $$ = $1 + $2; }
	;

/**
 * Valid forms of a select statement as parsed by Bash:
 * 1. select var in input; do ... statements ...; done
 * 2. select var in input; { ... statements ... }
 * 3. select var in; do ... statements ...; done
 * 4. select var in; { ... statements ... }
 * 5. select var; do ... statements ...; done
 * 6. select var; { ... statements ... }
 */
bash_select_statement:
	BASH_KEYWORD_SELECT WS bash_for_or_select_header DELIM maybe_whitespace BASH_KEYWORD_DO statements BASH_KEYWORD_DONE {
		std::string selectHeader = $3;

		std::cout << "Parsed bash select statement" << std::endl;

		$$ = "select " + selectHeader + " do\n... statements ...\ndone";
	}
	| BASH_KEYWORD_SELECT WS bash_for_or_select_header DELIM maybe_whitespace block {
		std::string selectHeader = $3;

		std::cout << "Parsed bash select statement with block" << std::endl;

		$$ = "select " + selectHeader + " {\n... statements ...\n}";
	}
	;

bash_for_or_select_header:
	bash_for_or_select_variable bash_for_or_select_maybe_in_something {
		std::string varName = $1;
		std::string inSomething = $2;

		std::cout << "Parsed bash for/select header: Variable='" << varName << "'" << std::endl;

		$$ = varName + inSomething;
	}
	;

bash_for_or_select_maybe_in_something:
	maybe_whitespace { 
		std::cout << "Parsed bash for/select header with no 'in'" << std::endl;
		$$ = "";
	}
	| WS BASH_KEYWORD_IN WS bash_for_or_select_input {
		std::string selectInput = $4;

		std::cout << "Parsed bash for/select header with input: Input='" << selectInput << "'" << std::endl;

		$$ = " in " + selectInput;
	}
	| WS BASH_KEYWORD_IN maybe_whitespace {
		std::cout << "Parsed bash for/select header with no input" << std::endl;
		$$ = " in";
	}
	;

bash_for_or_select_variable:
	IDENTIFIER {
		set_bash_for_or_select_variable_received(true);
		std::string varName = $1;
		std::cout << "Parsed bash for/select variable: Name='" << varName << "'" << std::endl;
		$$ = varName;
	}
	;

bash_for_or_select_input:
	valid_rvalue { $$ = $1; }
	| bash_for_or_select_input WS valid_rvalue { $$ = $1 + " " + $3; }
	| bash_for_or_select_input WS { $$ = $1; } /* Allow trailing whitespace */
	;

/**
 * Valid forms of a for statement as parsed by Bash:
 * 1. for var in input; do ... statements ...; done
 * 2. for var in input; { ... statements ... }
 * 3. for var in; do ... statements ...; done
 * 4. for var in; { ... statements ... }
 * 5. for var; do ... statements ...; done
 * 6. for var; { ... statements ... }
 */
bash_for_statement:
	BASH_KEYWORD_FOR WS bash_for_or_select_header DELIM maybe_whitespace BASH_KEYWORD_DO statements BASH_KEYWORD_DONE {
		std::string forHeader = $3;

		std::cout << "Parsed bash for statement" << std::endl;

		$$ = "for " + forHeader + " do\n... statements ...\ndone";
	}
	| BASH_KEYWORD_FOR WS bash_for_or_select_header DELIM maybe_whitespace block {
		std::string forHeader = $3;

		std::cout << "Parsed bash for statement with block" << std::endl;

		$$ = "for " + forHeader + " {\n... statements ...\n}";
	}
	;

/**
 * Valid forms of an arithmetic for statement as parsed by Bash:
 * 1. for ((expr; expr; expr)); do ... statements ...; done
 * 2. for ((expr; expr; expr)); { ... statements ... }
 * 3. for ((expr; expr; expr)) do ... statements ...; done
 * 4. for ((expr; expr; expr)) { ... statements ... }
 */
bash_arithmetic_for_statement:
	BASH_KEYWORD_FOR WS arithmetic_for_condition BASH_KEYWORD_DO statements BASH_KEYWORD_DONE {
		std::string forCondition = $3;

		std::cout << "Parsed bash arithmetic for statement" << std::endl;

		$$ = "for " + forCondition + " do\n... statements ...\ndone";
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition DELIM maybe_whitespace BASH_KEYWORD_DO statements BASH_KEYWORD_DONE {
		std::string forCondition = $3;

		std::cout << "Parsed bash arithmetic for statement" << std::endl;

		$$ = "for " + forCondition + " do\n... statements ...\ndone";
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition block {
		std::string forCondition = $3;

		std::cout << "Parsed bash arithmetic for statement with block" << std::endl;

		$$ = "for " + forCondition + " {\n... statements ...\n}";
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition DELIM maybe_whitespace block {
		std::string forCondition = $3;

		std::cout << "Parsed bash arithmetic for statement with block" << std::endl;

		$$ = "for " + forCondition + " {\n... statements ...\n}";
	}
	;

arithmetic_for_condition:
	ARITH_FOR_CONDITION_START arith_statement DELIM arith_statement DELIM arith_statement ARITH_FOR_CONDITION_END maybe_whitespace {
		std::string initExpr = $2;
		std::string condExpr = $4;
		std::string iterExpr = $6;

		std::cout << "Parsed bash arithmetic for condition: Init='" << initExpr << "', Condition='" << condExpr << "', Iteration='" << iterExpr << "'" << std::endl;

		$$ = "((" + initExpr + "; " + condExpr + "; " + iterExpr + "))";
	}
	;

/*
 * Expressions that are valid inside arithmetic for conditions:
 * - empty (no expression)
 * - valid rvalue
 * - object or shell variable assignment (e.g., i=0)
 * - Increment/decrement operators applied to object references or shell variables (e.g., i++, ++i, i--, --i)
 */
arith_statement:
	/* empty */ { $$ = ""; }
	| valid_rvalue { $$ = $1; }
	| IDENTIFIER_LVALUE { $$ = $1; }
	| object_reference_lvalue { $$ = $1; }
	| self_reference_lvalue { $$ = $1; }
	| object_assignment { $$ = $1; }
	| shell_variable_assignment { $$ = $1; }
	| increment_decrement_expression { $$ = $1; }
	| comparison_expression { $$ = $1; }
	;

increment_decrement_expression:
	arith_condition_term arith_operator {
		std::string ref = $1;
		std::string op = $2;

		std::cout << "Parsed increment/decrement expression: Reference='" << ref << "', Operator='" << op << "'" << std::endl;

		$$ = ref + op;
	}
	| arith_operator arith_condition_term {
		std::string op = $1;
		std::string ref = $2;

		std::cout << "Parsed increment/decrement expression: Operator='" << op << "', Reference='" << ref << "'" << std::endl;

		$$ = op + ref;
	}
	;

comparison_expression:
	arith_condition_term maybe_whitespace comparison_operator maybe_whitespace arith_condition_term {
		std::string leftTerm = $1;
		std::string compOp = $3;
		std::string rightTerm = $5;

		std::cout << "Parsed comparison expression: LeftTerm='" << leftTerm << "', Operator='" << compOp << "', RightTerm='" << rightTerm << "'" << std::endl;

		$$ = leftTerm + " " + compOp + " " + rightTerm;
	}
	;

comparison_operator:
	COMPARISON_OPERATOR { $$ = $1; }
	| LANGLE { $$ = "<"; }
	| RANGLE { $$ = ">"; }
	;

arith_condition_term:
	object_reference { $$ = $1; }
	| object_reference_lvalue { $$ = $1; }
	| self_reference { $$ = $1; }
	| self_reference_lvalue { $$ = $1; }
	| bash_variable { $$ = $1; }
	| IDENTIFIER_LVALUE { $$ = $1; }
	| IDENTIFIER { $$ = $1; }
	| INTEGER { $$ = $1; }
	| KEYWORD_NULLPTR { $$ = "0"; }
	;

arith_operator:
	INCREMENT_OPERATOR { $$ = "++"; }
	| DECREMENT_OPERATOR { $$ = "--"; }
	;

%%

namespace yy {
void parser::error(const std::string& m) {
	std::cerr << "Parse error: " << m << std::endl;
}
} // namespace yy
