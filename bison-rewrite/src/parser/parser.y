%language "c++"
%require "3.2"
%locations

%code requires {
/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */
#include <memory>
#include <cassert>
#include "../AST/Nodes/Nodes.h"
typedef std::shared_ptr<AST::ASTNode> ASTNodePtr;
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
	extern void set_bash_if_condition_received(bool received);
	extern void set_bash_while_or_until_condition_received(bool received);
	extern void set_parsed_assignment_operator(bool parsed);

	bool current_command_can_receive_lvalues = true;
}

%token <std::string> ESCAPED_CHAR WS DELIM
%token DOUBLEAMPERSAND DOUBLEPIPE PIPE

%token <std::string> SINGLEQUOTED_STRING

%token QUOTE_BEGIN QUOTE_END
%token <std::string> STRING_CONTENT

%token AT AT_LVALUE
%token KEYWORD_THIS KEYWORD_THIS_LVALUE KEYWORD_SUPER KEYWORD_SUPER_LVALUE
%token LBRACE RBRACE
%token <std::string> LANGLE RANGLE LANGLE_AMPERSAND RANGLE_AMPERSAND AMPERSAND_RANGLE
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
%token HEREDOC_CONTENT_START HERESTRING_START
%token <std::string> HEREDOC_START HEREDOC_DELIMITER HEREDOC_END

%token BASH_KEYWORD_CASE BASH_KEYWORD_IN BASH_CASE_PATTERN_DELIM BASH_CASE_PATTERN_TERMINATOR BASH_KEYWORD_ESAC
%token <std::string> BASH_CASE_BODY_BEGIN
%token BASH_KEYWORD_SELECT BASH_KEYWORD_FOR BASH_KEYWORD_DO BASH_KEYWORD_DONE
%token ARITH_FOR_CONDITION_START ARITH_FOR_CONDITION_END
%token INCREMENT_OPERATOR DECREMENT_OPERATOR
%token <std::string> INTEGER COMPARISON_OPERATOR

%token BASH_KEYWORD_IF BASH_KEYWORD_THEN BASH_KEYWORD_ELIF BASH_KEYWORD_ELSE BASH_KEYWORD_FI
%token BASH_KEYWORD_WHILE BASH_KEYWORD_UNTIL

%token EXCLAM
%token <std::string> EXPANSION_BEGIN PARAMETER_EXPANSION_CONTENT

%token <std::string> PROCESS_SUBSTITUTION_START
%token PROCESS_SUBSTITUTION_END

/* Handling unrecognized tokens */
%token <std::string> CATCHALL


%precedence CONCAT_STOP
%precedence LANGLE LANGLE_AMPERSAND RANGLE RANGLE_AMPERSAND AMPERSAND_RANGLE HEREDOC_START HERESTRING_START
%precedence IDENTIFIER INTEGER SINGLEQUOTED_STRING KEYWORD_NULLPTR
%precedence QUOTE_BEGIN
%precedence AT REF_START
%precedence KEYWORD_THIS KEYWORD_SUPER
%precedence AMPERSAND
%precedence DEREFERENCE_OPERATOR
%precedence BASH_VAR_START BASH_VAR
%precedence SUPERSHELL_START SUBSHELL_SUBSTITUTION_START DEPRECATED_SUBSHELL_START SUBSHELL_START PROCESS_SUBSTITUTION_START
%precedence LBRACE
%precedence CATCHALL

%left PIPE
%left DOUBLEAMPERSAND DOUBLEPIPE


/* Nonterminal types */
%type <ASTNodePtr> program
%type <std::vector<ASTNodePtr>> statements
%type <ASTNodePtr> statement

%type <AST::IncludeStatement::IncludeKeyword> include_keyword
%type <ASTNodePtr> include_statement

%type <ASTNodePtr> class_definition
%type <AST::AccessModifier> access_modifier access_modifier_keyword
%type <ASTNodePtr> datamember_declaration

%type <AST::MethodDefinition::Parameter> parameter
%type <std::vector<AST::MethodDefinition::Parameter>> maybe_parameter_list

%type <ASTNodePtr> method_definition constructor_definition destructor_definition

%type <ASTNodePtr> block supershell subshell_raw subshell_substitution dollar_subshell deprecated_subshell

%type <ASTNodePtr> object_instantiation instantiation_suffix
%type <ASTNodePtr> pointer_declaration pointer_declaration_preface
%type <ASTNodePtr> maybe_descend_object_hierarchy object_reference object_reference_lvalue self_reference self_reference_lvalue
%type <ASTNodePtr> object_address pointer_dereference pointer_dereference_rvalue pointer_dereference_lvalue

%type <ASTNodePtr> delete_statement new_statement

%type <ASTNodePtr> doublequoted_string quote_contents string_interpolation

%type <ASTNodePtr> valid_rvalue concatenatable_rvalue concatenated_rvalue
%type <ASTNodePtr> value_assignment maybe_default_value
%type <ASTNodePtr> object_assignment shell_variable_assignment

%type <ASTNodePtr> typeof_expression
%type <ASTNodePtr> dynamic_cast cast_target
%type <ASTNodePtr> bash_variable maybe_array_index maybe_parameter_expansion array_index

%type <ASTNodePtr> process_substitution
%type <ASTNodePtr> heredoc_body heredoc_content herestring
%type <ASTNodePtr> redirection
%type <ASTNodePtr> operative_command_element simple_command_element simple_command simple_pipeline simple_command_sequence
%type <ASTNodePtr> pipeline shell_command_sequence shell_command

%type <std::vector<ASTNodePtr>> command_redirections

%type <ASTNodePtr> bash_if_statement bash_if_condition bash_if_root_branch bash_if_else_branch
%type <std::vector<ASTNodePtr>> maybe_bash_if_else_branches

%type <ASTNodePtr> bash_for_statement bash_select_statement bash_for_or_select_header
%type <ASTNodePtr> bash_for_or_select_maybe_in_something bash_for_or_select_input
%type <ASTNodePtr> bash_case_statement bash_case_header bash_case_input bash_case_pattern bash_case_pattern_header
%type <std::vector<ASTNodePtr>> bash_case_body
%type <ASTNodePtr> bash_arithmetic_for_statement arithmetic_for_condition arith_statement increment_decrement_expression arith_condition_term comparison_expression

%type <ASTNodePtr> bash_while_statement bash_until_statement bash_while_or_until_condition

%type <std::string> maybe_include_type maybe_as_clause maybe_parent_class
%type <std::string> assignment_operator
%type <std::string> maybe_exclam
%type <std::string> maybe_hash
%type <std::string> heredoc_header
%type <std::string> bash_for_or_select_variable
%type <std::string> arith_operator comparison_operator
%type <std::string> redirection_operator
%type <std::string> logical_connective

%%

program: statements {
		std::shared_ptr<AST::Program> program = std::make_shared<AST::Program>();
		program->addChildren($1);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		program->setPosition(line_number, column_number);
		$$ = program;

		// Verification (Debug):
		std::cout << *program;
	}
	;

statements:
	/* empty */ { $$ = std::vector<std::shared_ptr<AST::ASTNode>>(); }
	| statements statement { $$ = std::move($1); if ($2) $$.push_back($2); }
	;

statement:
	DELIM {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| shell_command_sequence %prec CONCAT_STOP { $$ = $1; }
	| include_statement { $$ = $1; }
	| class_definition { $$ = $1; }
	| datamember_declaration {  $$ = $1; }
	| method_definition { $$ = $1; }
	| constructor_definition { $$ = $1; }
	| destructor_definition { $$ = $1; }
	| object_instantiation { $$ = $1; }
	| pointer_declaration { $$ = $1; }
	| delete_statement { $$ = $1; }
	;

shell_command_sequence:
	pipeline %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommandSequence>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| shell_command_sequence logical_connective maybe_whitespace pipeline {
		auto commandSequence = std::dynamic_pointer_cast<AST::BashCommandSequence>($1);
		commandSequence->addText(" " + $2 + " "); // Preserve connective with surrounding spaces
		commandSequence->addChild($4);
		$$ = commandSequence;
	}
	;

pipeline:
	shell_command %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashPipeline>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| pipeline PIPE maybe_whitespace shell_command {
		auto pipeline = std::dynamic_pointer_cast<AST::BashPipeline>($1);
		pipeline->addText(" | "); // Preserve pipe symbol
		pipeline->addChild($4);
		$$ = pipeline;
	}
	;

logical_connective:
	DOUBLEAMPERSAND { $$ = "&&"; }
	| DOUBLEPIPE { $$ = "||"; }
	;

shell_command:
	simple_command %prec CONCAT_STOP { current_command_can_receive_lvalues = true; $$ = $1; }
	| bash_case_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| bash_select_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| bash_for_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| bash_arithmetic_for_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| bash_if_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| bash_while_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| bash_until_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChildren($2);
		$$ = node;
	}
	| heredoc_body { $$ = $1; }
	;

command_redirections:
	/* empty */ %prec CONCAT_STOP { $$ = std::vector<ASTNodePtr>(); }
	| command_redirections redirection { $$ = std::move($1); $$.push_back($2); }
	| command_redirections WS redirection { $$ = std::move($1); $$.push_back($3); }
	;

simple_command_sequence:
	simple_pipeline %prec CONCAT_STOP {
		auto node = std::make_shared<AST::BashCommandSequence>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| simple_command_sequence logical_connective maybe_whitespace simple_pipeline {
		auto commandSequence = std::dynamic_pointer_cast<AST::BashCommandSequence>($1);
		commandSequence->addText(" " + $2 + " "); // Preserve connective with surrounding spaces
		commandSequence->addChild($4);
		$$ = commandSequence;
	}
	;

simple_pipeline:
	simple_command %prec CONCAT_STOP {
		current_command_can_receive_lvalues = true;
		auto node = std::make_shared<AST::BashPipeline>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| simple_pipeline PIPE maybe_whitespace simple_command {
		current_command_can_receive_lvalues = true;

		auto pipeline = std::dynamic_pointer_cast<AST::BashPipeline>($1);
		pipeline->addText(" | "); // Preserve pipe symbol
		pipeline->addChild($4);
		$$ = pipeline;
	}
	;

simple_command:
	simple_command_element {
		auto node = std::make_shared<AST::BashCommand>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| simple_command WS simple_command_element {
		auto command = std::dynamic_pointer_cast<AST::BashCommand>($1);
		command->addText(" "); // Preserve whitespace
		command->addChild($3);
		$$ = command;
	}
	| simple_command redirection {
		$1->addChild($2);
		$$ = $1;
	}
	;

simple_command_element:
	shell_variable_assignment { $$ = $1; }
	| object_assignment { $$ = $1; }
	| redirection { $$ = $1; }
	| operative_command_element { current_command_can_receive_lvalues = false; $$ = $1; }
	| valid_rvalue %prec CONCAT_STOP { current_command_can_receive_lvalues = false; $$ = $1; }
	| block { current_command_can_receive_lvalues = false; $$ = $1; }
	;

operative_command_element:
	IDENTIFIER_LVALUE {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| object_reference_lvalue { $$ = $1; }
	| self_reference_lvalue { $$ = $1; }
	| pointer_dereference_lvalue { $$ = $1; }
	;

redirection:
	redirection_operator maybe_whitespace valid_rvalue {
		// Lvalues can follow redirections iff we have not yet received the operative command element
		// E.g.:
		// >file var=value echo hi
		// 'var' is properly an lvalue here. By the time we hit '>file', we hadn't yet seen 'echo', so lvalues are still allowed
		// But:
		// echo hi >file var=value
		// In this case, 'var=value' is a simple string. Because we had already seen 'echo', lvalues are no longer allowed
		set_incoming_token_can_be_lvalue(current_command_can_receive_lvalues);

		auto node = std::make_shared<AST::BashRedirection>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setOperator($1);
		node->addChild($3);
		$$ = node;
	}
	| heredoc_header {
		set_incoming_token_can_be_lvalue(current_command_can_receive_lvalues);
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| herestring {
		set_incoming_token_can_be_lvalue(current_command_can_receive_lvalues);
		$$ = $1;
	}
	;

redirection_operator:
	LANGLE { $$ = $1; }
	| LANGLE RANGLE { $$ = $1 + $2; }
	| LANGLE_AMPERSAND { $$ = $1; }
	| RANGLE { $$ = $1; }
	| RANGLE RANGLE { $$ = $1 + $2; }
	| RANGLE_AMPERSAND { $$ = $1; }
	| RANGLE PIPE { $$ = $1 + "|"; }
	| AMPERSAND_RANGLE { $$ = $1; }
	| AMPERSAND_RANGLE RANGLE { $$ = $1 + $2; }
	;

block:
	LBRACE whitespace_or_delimiter statements RBRACE {
		auto node = std::make_shared<AST::Block>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->addChildren($3);
		$$ = node;
	}
	;

valid_rvalue:
	EMPTY_ASSIGNMENT {
		auto node = std::make_shared<AST::Rvalue>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addText("");
		$$ = node;
	}
	| new_statement {
		auto node = std::make_shared<AST::Rvalue>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| dynamic_cast {
		auto node = std::make_shared<AST::Rvalue>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| typeof_expression {
		auto node = std::make_shared<AST::Rvalue>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| concatenated_rvalue %prec CONCAT_STOP { $$ = $1; }
	;

concatenated_rvalue:
	concatenatable_rvalue %prec CONCAT_STOP {
		auto rvalue = std::make_shared<AST::Rvalue>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		rvalue->setPosition(line_number, column_number);
		rvalue->addChild($1);
		$$ = rvalue;
	}
	| concatenated_rvalue concatenatable_rvalue {
		auto rvalue = std::dynamic_pointer_cast<AST::Rvalue>($1);
		rvalue->addChild($2);
		$$ = rvalue;
	}
	;

concatenatable_rvalue:
	IDENTIFIER {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| INTEGER {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| SINGLEQUOTED_STRING {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| KEYWORD_NULLPTR {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText("0"); // Represent nullptr as 0
		$$ = node;
	}
	| CATCHALL { 
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| doublequoted_string { $$ = $1; }
	| object_reference { $$ = $1; }
	| self_reference { $$ = $1; }
	| object_address { $$ = $1; }
	| pointer_dereference_rvalue { $$ = $1; }
	| bash_variable { $$ = $1; }
	| supershell { $$ = $1; }
	| subshell_substitution { $$ = $1; }
	| subshell_raw { $$ = $1; }
	| process_substitution { $$ = $1; }
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
		AST::IncludeStatement::IncludeKeyword keyword = $1;
		AST::IncludeStatement::IncludeType type;
		if ($2 == "dynamic") {
			type = AST::IncludeStatement::IncludeType::DYNAMIC;
		} else {
			type = AST::IncludeStatement::IncludeType::STATIC;
		}

		AST::IncludeStatement::PathType pathType;
		std::string path = $3;
		if (path.front() == '<') {
			pathType = AST::IncludeStatement::PathType::ANGLEBRACKET;
		} else {
			pathType = AST::IncludeStatement::PathType::QUOTED;
		}

		path = path.substr(1, path.length() - 2); // Remove surrounding quotes or angle brackets

		std::string asPath = $4;
		if (!asPath.empty()) asPath = asPath.substr(1, asPath.length() - 2); // Remove surrounding quotes

		auto node = std::make_shared<AST::IncludeStatement>();

		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setKeyword(keyword);
		node->setType(type);
		node->setPathType(pathType);
		node->setPath(path);
		node->setAsPath(asPath);

		$$ = node;
	}
	;

include_keyword:
	KEYWORD_INCLUDE { $$ = AST::IncludeStatement::IncludeKeyword::INCLUDE; }
	| KEYWORD_INCLUDE_ONCE { $$ = AST::IncludeStatement::IncludeKeyword::INCLUDE_ONCE; }
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
	AT_LVALUE IDENTIFIER instantiation_suffix {
		if ($3 == nullptr) {
			// Not an object instantiation, but an lvalue object reference
			auto node = std::make_shared<AST::ObjectReference>();
			uint32_t line_number = @1.begin.line;
			uint32_t column_number = @1.begin.column;
			node->setPosition(line_number, column_number);

			node->setIdentifier($2);
			node->setLvalue(true);
			node->setAddressOf(false);
			node->setPointerDereference(false);
			node->setSelfReference(false);

			$$ = node;
		} else {
			// Use the ObjectInstantiation node returned by instantiation_suffix
			auto node = std::dynamic_pointer_cast<AST::ObjectInstantiation>($3);
			uint32_t line_number = @1.begin.line;
			uint32_t column_number = @1.begin.column;
			node->setPosition(line_number, column_number);
			node->setType($2);
			$$ = node;
		}
	}
	;

instantiation_suffix:
	WS IDENTIFIER maybe_default_value {
		auto node = std::make_shared<AST::ObjectInstantiation>();
		node->setIdentifier($2);
		node->addChild($3);
		$$ = node;
	}
	| WS { $$ = nullptr; }
	;

pointer_declaration:
	pointer_declaration_preface WS IDENTIFIER_LVALUE maybe_default_value {
		auto node = std::dynamic_pointer_cast<AST::PointerDeclaration>($1);
		node->setIdentifier($3);
		node->addChild($4);

		$$ = node;
	}
	;

pointer_declaration_preface:
	AT_LVALUE IDENTIFIER ASTERISK {
		set_incoming_token_can_be_lvalue(true); // The following identifier should be an lvalue, let the lexer know
		
		auto node = std::make_shared<AST::PointerDeclaration>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setType($2);

		$$ = node;
	}

new_statement:
	KEYWORD_NEW WS IDENTIFIER {
		auto node = std::make_shared<AST::NewStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setType($3);

		$$ = node;
	}
	;

delete_statement:
	KEYWORD_DELETE WS object_reference {
		auto node = std::make_shared<AST::DeleteStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->addChild($3);

		$$ = node;
	}
	|
	KEYWORD_DELETE WS self_reference {
		auto node = std::make_shared<AST::DeleteStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->addChild($3);

		$$ = node;
	}
	;

class_definition:
	KEYWORD_CLASS WS IDENTIFIER maybe_parent_class block {
		auto node = std::make_shared<AST::ClassDefinition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setClassName($3);
		node->setParentClassName($4);
		node->addChild($5);
		$$ = node;
	}
	;

maybe_parent_class:
	whitespace_or_delimiter { $$ = ""; }
	| WS COLON WS IDENTIFIER whitespace_or_delimiter { $$ = $4; }
	;

datamember_declaration:
	access_modifier IDENTIFIER_LVALUE maybe_default_value DELIM {
		auto node = std::make_shared<AST::DatamemberDeclaration>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setAccessModifier($1);
		node->setIdentifier($2);
		node->addChild($3);

		$$ = node;
	}
	| access_modifier object_instantiation {
		auto node = std::make_shared<AST::DatamemberDeclaration>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		if (std::dynamic_pointer_cast<AST::ObjectInstantiation>($2) == nullptr) {
			// ERROR: `@public @className` is not sufficient to declare a datamember
			// TBD: Handle this error properly later
		}

		node->setAccessModifier($1);
		node->addChild($2);

		$$ = node;
	}
	| access_modifier pointer_declaration {
		auto node = std::make_shared<AST::DatamemberDeclaration>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setAccessModifier($1);
		node->addChild($2);

		$$ = node;
	}
	;

access_modifier:
	access_modifier_keyword WS {
		$$ = $1;
	}
	;

access_modifier_keyword:
	KEYWORD_PUBLIC { $$ = AST::AccessModifier::PUBLIC; }
	| KEYWORD_PRIVATE { $$ = AST::AccessModifier::PRIVATE; }
	| KEYWORD_PROTECTED { $$ = AST::AccessModifier::PROTECTED; }
	;

maybe_default_value:
	maybe_whitespace { $$ = nullptr; }
	| value_assignment { $$ = $1; }
	;

value_assignment:
	assignment_operator valid_rvalue {
		auto node = std::make_shared<AST::ValueAssignment>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setOperator($1);
		node->addChild($2);

		$$ = node;
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
		auto node = std::make_shared<AST::MethodDefinition>();
		uint32_t line_number = @2.begin.line;
		uint32_t column_number = @2.begin.column;
		node->setPosition(line_number, column_number);

		node->setAccessModifier($1);
		node->setName($4);
		node->addParameters($6);
		node->addChild($7);

		node->setVirtual(false);

		$$ = node;
	}
	| KEYWORD_VIRTUAL WS access_modifier KEYWORD_METHOD WS IDENTIFIER WS maybe_parameter_list block {
		auto node = std::make_shared<AST::MethodDefinition>();
		uint32_t line_number = @2.begin.line;
		uint32_t column_number = @2.begin.column;
		node->setPosition(line_number, column_number);

		node->setAccessModifier($3);
		node->setName($6);
		node->addParameters($8);
		node->addChild($9);

		node->setVirtual(true);

		$$ = node;
	}
	;

maybe_parameter_list:
	/* empty */ { $$ = std::vector<AST::MethodDefinition::Parameter>(); }
	| maybe_parameter_list parameter { $$ = std::move($1); $$.push_back($2); }
	;

parameter:
	IDENTIFIER WS {
		AST::MethodDefinition::Parameter param;
		param.type = std::nullopt;
		param.name = $1;
		param.pointer = false;
		$$ = param;
	}
	| AT IDENTIFIER ASTERISK WS IDENTIFIER WS {
		AST::MethodDefinition::Parameter param;
		param.type = $2;
		param.name = $5;
		param.pointer = true;
		$$ = param;
	}
	| AT IDENTIFIER WS IDENTIFIER WS {
		/* Actually invalid, but error handling should come later when traversing the AST */
		/* Invalid because methods cannot take non-primitive parameters */
		AST::MethodDefinition::Parameter param;
		param.type = $2;
		param.name = $4;
		param.pointer = false;
		$$ = param;
	}
	;

constructor_definition:
	KEYWORD_CONSTRUCTOR WS block {
		auto node = std::make_shared<AST::ConstructorDefinition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3);
		$$ = node;
	}
	;

destructor_definition:
	KEYWORD_DESTRUCTOR WS block {
		auto node = std::make_shared<AST::DestructorDefinition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3);
		$$ = node;
	}
	;

doublequoted_string:
	QUOTE_BEGIN quote_contents QUOTE_END {
		auto node = std::dynamic_pointer_cast<AST::DoublequotedString>($2);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		$$ = node;
	}
	;

quote_contents:
	/* empty */ { $$ = std::make_shared<AST::DoublequotedString>(); }
	| quote_contents STRING_CONTENT { $$ = $1; std::dynamic_pointer_cast<AST::DoublequotedString>($$)->addText($2); }
	| quote_contents string_interpolation { $$ = $1; std::dynamic_pointer_cast<AST::DoublequotedString>($$)->addChild($2); }
	;

string_interpolation:
	object_reference { $$ = $1; }
	| self_reference { $$ = $1; }
	| object_address { $$ = $1; }
	| pointer_dereference { $$ = $1; }
	| supershell { $$ = $1; }
	| subshell_substitution { $$ = $1; }
	| bash_variable { $$ = $1; }
	;

object_reference:
	AT IDENTIFIER maybe_descend_object_hierarchy {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($3);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier($2);
		node->setLvalue(false);
		node->setSelfReference(false);

		$$ = node;
	}
	| REF_START maybe_hash IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($4);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier($3);
		node->setLvalue(false);
		node->setSelfReference(false);

		if (!$2.empty()) {
			node->setHasHashkey(true);
		}
		
		node->addChild($5);

		$$ = node;
	}
	;

object_reference_lvalue:
	AT_LVALUE IDENTIFIER maybe_descend_object_hierarchy {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($3);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier($2);
		node->setLvalue(true);
		node->setSelfReference(false);

		$$ = node;
	}
	| REF_START_LVALUE maybe_hash IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($4);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier($3);
		node->setLvalue(true);
		node->setSelfReference(false);

		if (!$2.empty()) {
			node->setHasHashkey(true);
		}

		node->addChild($5);

		$$ = node;
	}
	;

self_reference:
	KEYWORD_THIS maybe_descend_object_hierarchy {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("this");
		node->setLvalue(false);
		node->setSelfReference(true);

		$$ = node;
	}
	| REF_START maybe_hash KEYWORD_THIS maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($4);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("this");
		node->setLvalue(false);
		node->setSelfReference(true);

		if (!$2.empty()) {
			node->setHasHashkey(true);
		}

		node->addChild($5);

		$$ = node;
	}
	| KEYWORD_SUPER maybe_descend_object_hierarchy {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("super");
		node->setLvalue(false);
		node->setSelfReference(true);

		$$ = node;
	}
	| REF_START maybe_hash KEYWORD_SUPER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($4);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("super");
		node->setLvalue(false);
		node->setSelfReference(true);

		if (!$2.empty()) {
			node->setHasHashkey(true);
		}

		node->addChild($5);

		$$ = node;
	}
	;

self_reference_lvalue:
	KEYWORD_THIS_LVALUE maybe_descend_object_hierarchy {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("this");
		node->setLvalue(true);
		node->setSelfReference(true);

		$$ = node;
	}
	| REF_START_LVALUE maybe_hash KEYWORD_THIS maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($4);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("this");
		node->setLvalue(true);
		node->setSelfReference(true);

		if (!$2.empty()) {
			node->setHasHashkey(true);
		}

		node->addChild($5);

		$$ = node;
	}
	| KEYWORD_SUPER_LVALUE maybe_descend_object_hierarchy {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("super");
		node->setLvalue(true);
		node->setSelfReference(true);

		$$ = node;
	}
	| REF_START_LVALUE maybe_hash KEYWORD_SUPER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($4);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setIdentifier("super");
		node->setLvalue(true);
		node->setSelfReference(true);

		if (!$2.empty()) {
			node->setHasHashkey(true);
		}

		node->addChild($5);

		$$ = node;
	}
	;

maybe_descend_object_hierarchy:
	/* empty */ { $$ = std::make_shared<AST::ObjectReference>(); }
	| maybe_descend_object_hierarchy DOT IDENTIFIER {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($1);
		node->addIdentifier($3);
		$$ = node;
	}
	;

maybe_array_index:
	/* empty */ { $$ = nullptr; }
	| ARRAY_INDEX_START array_index ARRAY_INDEX_END {
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		$2->setPosition(line_number, column_number);
		$$ = $2;
	}
	;

array_index:
	valid_rvalue {
		auto node = std::make_shared<AST::ArrayIndex>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| AT {
		// '@' is a valid array index, as in ${array[@]}
		auto node = std::make_shared<AST::ArrayIndex>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		auto atNode = std::make_shared<AST::RawText>();
		atNode->setPosition(line_number, column_number);
		atNode->setText("@");
		node->addChild(atNode);
		$$ = node;
	}
	;

maybe_exclam:
	/* empty */ { $$ = ""; }
	| EXCLAM { $$ = "!"; }
	;

maybe_hash:
	/* empty */ { $$ = ""; }
	| HASH { $$ = "#"; }
	;

bash_variable:
	BASH_VAR_START maybe_exclam maybe_hash IDENTIFIER maybe_array_index maybe_parameter_expansion BASH_VAR_END {
		auto node = std::make_shared<AST::BashVariable>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		std::string text = $2 + $3 + $4; // Remember to re-enclose in ${...} later
		node->setText(text);
		node->addChild($5);
		node->addChild($6);
		$$ = node;
	}
	| BASH_VAR {
		auto node = std::make_shared<AST::BashVariable>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setText($1); // Just the simple $VAR form
		$$ = node;
	}
	;

maybe_parameter_expansion:
	/* empty */ { $$ = nullptr; }
	| EXPANSION_BEGIN valid_rvalue {
		auto node = std::make_shared<AST::ParameterExpansion>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setExpansionBegin($1);
		node->addChild($2);
		$$ = node;
	}
	| EXPANSION_BEGIN PARAMETER_EXPANSION_CONTENT {
		auto node = std::make_shared<AST::ParameterExpansion>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setExpansionBegin($1);
		auto contentNode = std::make_shared<AST::RawText>();
		contentNode->setPosition(@2.begin.line, @2.begin.column);
		contentNode->setText($2);
		node->addChild(contentNode);
		$$ = node;
	}
	;

dynamic_cast:
	KEYWORD_DYNAMIC_CAST LANGLE cast_target RANGLE WS valid_rvalue {
		auto node = std::make_shared<AST::DynamicCast>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3); // cast_target
		node->addChild($6); // valid_rvalue
		$$ = node;
	}
	;

cast_target:
	IDENTIFIER {
		auto node = std::make_shared<AST::DynamicCastTarget>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		auto rawTextTarget = std::make_shared<AST::RawText>();
		rawTextTarget->setPosition(line_number, column_number);
		rawTextTarget->setText($1);
		node->addChild(rawTextTarget);
		$$ = node;
	}
	| bash_variable {
		auto node = std::make_shared<AST::DynamicCastTarget>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| object_reference {
		auto node = std::make_shared<AST::DynamicCastTarget>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| self_reference {
		auto node = std::make_shared<AST::DynamicCastTarget>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	;

object_assignment:
	object_reference_lvalue value_assignment {
		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		auto node = std::make_shared<AST::ObjectAssignment>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChild($2);
		$$ = node;
	}
	| self_reference_lvalue value_assignment {
		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		auto node = std::make_shared<AST::ObjectAssignment>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChild($2);
		$$ = node;
	}
	| pointer_dereference_lvalue value_assignment {
		set_incoming_token_can_be_lvalue(true); // Lvalues can follow assignments

		auto node = std::make_shared<AST::ObjectAssignment>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addChild($2);
		$$ = node;
	}
	;

shell_variable_assignment:
	IDENTIFIER_LVALUE value_assignment {
		auto node = std::make_shared<AST::PrimitiveAssignment>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setIdentifier($1);
		node->addChild($2);
		$$ = node;
	}
	;

object_address:
	AMPERSAND object_reference {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '&' token

		node->setAddressOf(true);

		$$ = node;
	}
	| AMPERSAND self_reference {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '&' token

		node->setAddressOf(true);

		$$ = node;
	}
	;

pointer_dereference:
	pointer_dereference_rvalue { $$ = $1; }
	| pointer_dereference_lvalue { $$ = $1; }
	;

pointer_dereference_rvalue:
	DEREFERENCE_OPERATOR object_reference {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setPointerDereference(true);
		$$ = node;
	}
	| DEREFERENCE_OPERATOR self_reference {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setPointerDereference(true);
		$$ = node;
	}
	;

pointer_dereference_lvalue:
	DEREFERENCE_OPERATOR object_reference_lvalue {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setPointerDereference(true);
		$$ = node;
	}
	| DEREFERENCE_OPERATOR self_reference_lvalue {
		auto node = std::dynamic_pointer_cast<AST::ObjectReference>($2);
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setPointerDereference(true);
		$$ = node;
	}
	;

typeof_expression:
	KEYWORD_TYPEOF WS valid_rvalue {
		auto node = std::make_shared<AST::TypeofExpression>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3);
		$$ = node;
	}

supershell:
	SUPERSHELL_START statements SUPERSHELL_END {
		auto node = std::make_shared<AST::Supershell>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChildren($2);
		$$ = node;
	}
	;

subshell_raw:
	SUBSHELL_START statements SUBSHELL_END {
		auto node = std::make_shared<AST::RawSubshell>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChildren($2);
		$$ = node;
	}
	;

subshell_substitution:
	dollar_subshell { $$ = $1; }
	| deprecated_subshell { $$ = $1; }
	;

dollar_subshell:
	SUBSHELL_SUBSTITUTION_START statements SUBSHELL_SUBSTITUTION_END {
		auto node = std::make_shared<AST::SubshellSubstitution>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChildren($2);
		$$ = node;
	}
	;

deprecated_subshell:
	DEPRECATED_SUBSHELL_START statements DEPRECATED_SUBSHELL_END {
		// NOTE: The nesting depth is stored as the semantic value of the DEPRECATED_SUBSHELL_START token
		assert($1 == $3 && "Mismatched deprecated subshell nesting depths!");

		auto node = std::make_shared<AST::SubshellSubstitution>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChildren($2);
		$$ = node;
	}
	;

process_substitution:
	PROCESS_SUBSTITUTION_START statements PROCESS_SUBSTITUTION_END {
		auto node = std::make_shared<AST::ProcessSubstitution>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setSubstitutionStart($1);
		node->addChildren($2);
		$$ = node;
	}
	;

heredoc_header:
	HEREDOC_START HEREDOC_DELIMITER {
		$$ = $1;
	}
	;

heredoc_body:
	HEREDOC_CONTENT_START heredoc_content HEREDOC_END {
		auto node = std::dynamic_pointer_cast<AST::HeredocBody>($2);
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setDelimiter($3);
		$$ = node;
	}
	;

heredoc_content:
	/* empty */ { $$ = std::make_shared<AST::HeredocBody>(); }
	| heredoc_content STRING_CONTENT {
		auto node = std::dynamic_pointer_cast<AST::HeredocBody>($1);
		node->addText($2);
		$$ = node;
	}
	| heredoc_content string_interpolation {
		auto node = std::dynamic_pointer_cast<AST::HeredocBody>($1);
		node->addChild($2);
		$$ = node;
		}
	;

herestring:
	HERESTRING_START maybe_whitespace valid_rvalue {
		auto node = std::make_shared<AST::HereString>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3);
		$$ = node;
	}
	;

bash_case_statement:
	BASH_KEYWORD_CASE WS bash_case_header bash_case_body BASH_KEYWORD_ESAC {
		auto node = std::make_shared<AST::BashCaseStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3); // bash_case_input
		node->addChildren($4); // bash_case_body
		$$ = node;
	}
	;

bash_case_header:
	bash_case_input WS BASH_KEYWORD_IN BASH_CASE_BODY_BEGIN { $$ = $1; }
	;

bash_case_input:
	valid_rvalue {
		set_bash_case_input_received(true);
		auto node = std::make_shared<AST::BashCaseInput>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	;

bash_case_body:
	/* empty */ { $$ = std::vector<ASTNodePtr>(); }
	| bash_case_body bash_case_pattern { $$ = std::move($1); $$.push_back($2); }
	;

bash_case_pattern:
	bash_case_pattern_header BASH_CASE_PATTERN_DELIM statements BASH_CASE_PATTERN_TERMINATOR {
		auto node = std::make_shared<AST::BashCasePattern>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1); // pattern header

		auto action = std::make_shared<AST::BashCasePatternAction>();
		action->setPosition(@3.begin.line, @3.begin.column);
		action->addChildren($3); // statements

		node->addChild(action);
		$$ = node;
	}
	;

bash_case_pattern_header:
	/* empty */ { $$ = std::make_shared<AST::BashCasePatternHeader>(); }
	| bash_case_pattern_header STRING_CONTENT {
		auto node = std::dynamic_pointer_cast<AST::BashCasePatternHeader>($1);
		node->addText($2);
		$$ = node;
	}
	| bash_case_pattern_header string_interpolation {
		auto node = std::dynamic_pointer_cast<AST::BashCasePatternHeader>($1);
		node->addChild($2);
		$$ = node;
	}
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
		auto forStatement = std::dynamic_pointer_cast<AST::BashForStatement>($3);
		auto selectStatement = std::make_shared<AST::BashSelectStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		selectStatement->setPosition(line_number, column_number);
		// Earlier, we assumed it was a 'for' statement by default
		if (!forStatement) {
			// This should not happen, but just in case
			selectStatement = std::dynamic_pointer_cast<AST::BashSelectStatement>($3);
		} else {
			selectStatement->setVariable(forStatement->VARIABLE());
			selectStatement->addChildren(forStatement->getChildren());
		}
		selectStatement->addChildren($7);
		$$ = selectStatement;
	}
	| BASH_KEYWORD_SELECT WS bash_for_or_select_header DELIM maybe_whitespace block {
		auto forStatement = std::dynamic_pointer_cast<AST::BashForStatement>($3);
		auto selectStatement = std::make_shared<AST::BashSelectStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		selectStatement->setPosition(line_number, column_number);
		// Earlier, we assumed it was a 'for' statement by default
		if (!forStatement) {
			// This should not happen, but just in case
			selectStatement = std::dynamic_pointer_cast<AST::BashSelectStatement>($3);
		} else {
			selectStatement->setVariable(forStatement->VARIABLE());
			selectStatement->addChildren(forStatement->getChildren());
		}
		selectStatement->addChild($6);
		$$ = selectStatement;
	}
	;

bash_for_or_select_header:
	bash_for_or_select_variable bash_for_or_select_maybe_in_something {
		// Here, we assume that it's a 'for' statement by default
		// 'for' is much more common than 'select'
		// If it winds up being 'select' instead, the AST node will be updated later
		// When we're in the bash_select_statement rule
		auto forStatement = std::make_shared<AST::BashForStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		forStatement->setPosition(line_number, column_number);
		forStatement->setVariable($1);
		forStatement->addChild($2);
		$$ = forStatement;
	}
	;

bash_for_or_select_maybe_in_something:
	maybe_whitespace { $$ = nullptr; }
	| WS BASH_KEYWORD_IN WS bash_for_or_select_input {
		auto inCondition = std::dynamic_pointer_cast<AST::BashInCondition>($4);
		inCondition->setPosition(@2.begin.line, @2.begin.column); // Move position to 'in' token
		$$ = inCondition;
	}
	| WS BASH_KEYWORD_IN maybe_whitespace {
		auto node = std::make_shared<AST::BashInCondition>();
		uint32_t line_number = @2.begin.line;
		uint32_t column_number = @2.begin.column;
		node->setPosition(line_number, column_number);
		$$ = node; // 'in' with no input, valid in Bash
	}
	;

bash_for_or_select_variable:
	IDENTIFIER {
		set_bash_for_or_select_variable_received(true);
		$$ = $1;
	}
	;

bash_for_or_select_input:
	valid_rvalue {
		auto node = std::make_shared<AST::BashInCondition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| bash_for_or_select_input WS valid_rvalue {
		auto inCondition = std::dynamic_pointer_cast<AST::BashInCondition>($1);
		inCondition->addText(" "); // Preserve whitespace between items
		inCondition->addChild($3);
		$$ = inCondition;
	}
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
		auto forStatement = std::dynamic_pointer_cast<AST::BashForStatement>($3);
		if (!forStatement) {
			// This should not happen, but just in case
			auto selectStatement = std::dynamic_pointer_cast<AST::BashSelectStatement>($3);
			forStatement = std::make_shared<AST::BashForStatement>();
			forStatement->setVariable(selectStatement->VARIABLE());
			forStatement->addChildren(selectStatement->getChildren());
		}
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		forStatement->setPosition(line_number, column_number);
		forStatement->addChildren($7);
		$$ = forStatement;
	}
	| BASH_KEYWORD_FOR WS bash_for_or_select_header DELIM maybe_whitespace block {
		auto forStatement = std::dynamic_pointer_cast<AST::BashForStatement>($3);
		if (!forStatement) {
			// This should not happen, but just in case
			auto selectStatement = std::dynamic_pointer_cast<AST::BashSelectStatement>($3);
			forStatement = std::make_shared<AST::BashForStatement>();
			forStatement->setVariable(selectStatement->VARIABLE());
			forStatement->addChildren(selectStatement->getChildren());
		}
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		forStatement->setPosition(line_number, column_number);
		forStatement->addChild($6);
		$$ = forStatement;
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
		auto node = std::make_shared<AST::BashArithmeticForStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3); // for condition
		node->addChildren($5); // statements
		$$ = node;
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition DELIM maybe_whitespace BASH_KEYWORD_DO statements BASH_KEYWORD_DONE {
		auto node = std::make_shared<AST::BashArithmeticForStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3); // for condition
		node->addChildren($7); // statements
		$$ = node;
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition block {
		auto node = std::make_shared<AST::BashArithmeticForStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3); // for condition
		node->addChild($4); // block
		$$ = node;
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition DELIM maybe_whitespace block {
		auto node = std::make_shared<AST::BashArithmeticForStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($3); // for condition
		node->addChild($6); // block
		$$ = node;
	}
	;

arithmetic_for_condition:
	ARITH_FOR_CONDITION_START arith_statement DELIM arith_statement DELIM arith_statement ARITH_FOR_CONDITION_END maybe_whitespace {
		auto node = std::make_shared<AST::BashArithmeticForCondition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($2); // first expression
		node->addChild($4); // second expression
		node->addChild($6); // third expression
		$$ = node;
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
	/* empty */ { $$ = std::make_shared<AST::BashArithmeticStatement>(); }
	| valid_rvalue {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| IDENTIFIER_LVALUE {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		auto rawText = std::make_shared<AST::RawText>();
		rawText->setPosition(line_number, column_number);
		rawText->setText($1);
		node->addChild(rawText);
		$$ = node;
	}
	| object_reference_lvalue {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| self_reference_lvalue {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| object_assignment {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| shell_variable_assignment {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	| increment_decrement_expression { $$ = $1; }
	| comparison_expression { $$ = $1; }
	;

increment_decrement_expression:
	arith_condition_term arith_operator {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addText($2);
		$$ = node;
	}
	| arith_operator arith_condition_term {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addText($1);
		node->addChild($2);
		$$ = node;
	}
	;

comparison_expression:
	arith_condition_term maybe_whitespace comparison_operator maybe_whitespace arith_condition_term {
		auto node = std::make_shared<AST::BashArithmeticStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		node->addText($3);
		node->addChild($5);
		$$ = node;
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
	| self_reference { $$ = $1;}
	| self_reference_lvalue {$$ = $1; }
	| bash_variable { $$ = $1; }
	| IDENTIFIER_LVALUE {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| IDENTIFIER {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| INTEGER {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText($1);
		$$ = node;
	}
	| KEYWORD_NULLPTR {
		auto node = std::make_shared<AST::RawText>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setText("0");
		$$ = node;
	}
	;

arith_operator:
	INCREMENT_OPERATOR { $$ = "++"; }
	| DECREMENT_OPERATOR { $$ = "--"; }
	;

bash_if_statement:
	bash_if_root_branch maybe_bash_if_else_branches BASH_KEYWORD_FI {
		auto node = std::dynamic_pointer_cast<AST::BashIfStatement>($1);
		node->addChildren($2); // elif / else branches
		$$ = node;
	}
	;

bash_if_root_branch:
	BASH_KEYWORD_IF bash_if_condition DELIM maybe_whitespace BASH_KEYWORD_THEN maybe_whitespace statements {
		auto node = std::make_shared<AST::BashIfStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->addChild($2); // condition

		auto rootBranch = std::make_shared<AST::BashIfRootBranch>();
		rootBranch->setPosition(@5.begin.line, @5.begin.column);
		rootBranch->addChildren($7); // statements
		
		node->addChild(rootBranch);
		$$ = node;
	}
	;

bash_if_condition:
	simple_command_sequence {
		set_bash_if_condition_received(true);
		auto node = std::make_shared<AST::BashIfCondition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	;

maybe_bash_if_else_branches:
	/* empty */ { $$ = std::vector<ASTNodePtr>(); }
	| maybe_bash_if_else_branches bash_if_else_branch { $$ = std::move($1); $$.push_back($2); }
	;

bash_if_else_branch:
	BASH_KEYWORD_ELIF bash_if_condition DELIM maybe_whitespace BASH_KEYWORD_THEN maybe_whitespace statements {
		auto node = std::make_shared<AST::BashIfElseBranch>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->addChild($2); // condition
		node->addChildren($7); // statements

		$$ = node;
	}
	| BASH_KEYWORD_ELSE DELIM maybe_whitespace statements {
		auto node = std::make_shared<AST::BashIfElseBranch>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		// 'else' branch has no condition
		node->addChildren($4); // statements

		$$ = node;
	}
	;

bash_while_statement:
	BASH_KEYWORD_WHILE bash_while_or_until_condition DELIM maybe_whitespace BASH_KEYWORD_DO maybe_whitespace statements BASH_KEYWORD_DONE {
		auto node = std::make_shared<AST::BashWhileStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($2); // condition
		node->addChildren($7); // statements
		$$ = node;
	}
	;

bash_until_statement:
	BASH_KEYWORD_UNTIL bash_while_or_until_condition DELIM maybe_whitespace BASH_KEYWORD_DO maybe_whitespace statements BASH_KEYWORD_DONE {
		auto node = std::make_shared<AST::BashUntilStatement>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($2); // condition
		node->addChildren($7); // statements
		$$ = node;
	}
	;

bash_while_or_until_condition:
	simple_command_sequence {
		set_bash_while_or_until_condition_received(true);
		auto node = std::make_shared<AST::BashWhileOrUntilCondition>();
		uint32_t line_number = @1.begin.line;
		uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->addChild($1);
		$$ = node;
	}
	;

%%

namespace yy {
void parser::error(const location_type& loc, const std::string& m) {
	std::cerr << loc << ": " << m << std::endl;
}
} // namespace yy
