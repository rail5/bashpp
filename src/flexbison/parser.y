%language "c++"
%require "3.2"
%locations

%code requires {
/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <memory>
#include <cassert>
#include <filesystem>
#include <AST/Nodes/Nodes.h>
#include <include/ParserPosition.h>
#include <error/SyntaxError.h>
typedef std::unique_ptr<bpp::AST::ASTNode> ASTNodePtr;
typedef void* yyscan_t;
}

%{
#include <iostream>
#include <memory>
void yyerror(const char *s);
%}

%lex-param { yyscan_t yyscanner }
%parse-param { std::unique_ptr<bpp::AST::Program>& program } { bool& current_command_can_receive_lvalues } { const std::vector<std::filesystem::path>& include_chain } { std::vector<bpp::ErrorHandling::ParserError>& errors } { bool& lsp_mode } { yyscan_t yyscanner }

%define parse.error verbose

%define api.token.constructor
%define api.value.type variant
%define api.location.type { ParserLocation }
%code {
	#include "parser.tab.hpp"

	yy::parser::symbol_type yylex(yyscan_t yyscanner);

	// The following 'set' functions are used to send signals to the lexer about the current parsing context
	extern void set_incoming_token_can_be_lvalue(bool canBeLvalue, yyscan_t yyscanner);
	extern void set_bash_case_input_received(bool received, yyscan_t yyscanner);
	extern void set_bash_for_or_select_variable_received(bool received, yyscan_t yyscanner);
	extern void set_bash_if_condition_received(bool received, yyscan_t yyscanner);
	extern void set_bash_while_or_until_condition_received(bool received, yyscan_t yyscanner);
	extern void set_parsed_assignment_operator(bool parsed, yyscan_t yyscanner);
	extern void set_received_local_keyword(bool received, yyscan_t yyscanner);

	bool is_only_input_redirection(const std::vector<ASTNodePtr>& statements) {
		// This is awful.
		// And fragile. The anticipated AST structure is:
		// BashCommandSequence
		//   BashPipeline
		//     BashCommand
		//       BashRedirection
		// If that structure changes, this will break.
		if (statements.size() != 1) return false;
		if (statements[0]->getType() != bpp::AST::NodeType::BashCommandSequence) return false;
		auto* commandSequence = static_cast<bpp::AST::BashCommandSequence*>(statements[0].get());

		if (commandSequence->getChildren().size() != 1) return false;
		if (commandSequence->getChildren()[0]->getType() != bpp::AST::NodeType::BashPipeline) return false;
		auto* pipeline = static_cast<bpp::AST::BashPipeline*>(commandSequence->getChildren()[0].get());

		if (pipeline->getChildren().size() != 1) return false;
		if (pipeline->getChildren()[0]->getType() != bpp::AST::NodeType::BashCommand) return false;
		auto* command = static_cast<bpp::AST::BashCommand*>(pipeline->getChildren()[0].get());

		if (command->getChildren().size() != 1) return false;
		if (command->getChildren()[0]->getType() != bpp::AST::NodeType::BashRedirection) return false;
		auto* redirection = static_cast<bpp::AST::BashRedirection*>(command->getChildren()[0].get());

		if (!redirection->OPERATOR().getValue().contains('<')) return false;

		return true;
	}

	bool exec_received_A_option = false;
}

%token <bpp::AST::Token<std::string>> ESCAPED_CHAR WS DELIM
%token DOUBLEAMPERSAND DOUBLEPIPE PIPE

%token <bpp::AST::Token<std::string>> SINGLEQUOTED_STRING

%token QUOTE_BEGIN QUOTE_END
%token <bpp::AST::Token<std::string>> STRING_CONTENT

%token AT AT_LVALUE
%token KEYWORD_THIS KEYWORD_THIS_LVALUE KEYWORD_SUPER KEYWORD_SUPER_LVALUE
%token LBRACE RBRACE
%token <bpp::AST::Token<std::string>> LANGLE RANGLE LANGLE_AMPERSAND RANGLE_AMPERSAND AMPERSAND_RANGLE
%token COLON PLUS_EQUALS EQUALS ASTERISK DEREFERENCE_OPERATOR AMPERSAND DOT
%token EMPTY_ASSIGNMENT

%token KEYWORD_INCLUDE KEYWORD_INCLUDE_ALWAYS KEYWORD_AS KEYWORD_DYNAMIC_CAST
%token <bpp::AST::Token<std::string>> INCLUDE_TYPE INCLUDE_PATH

%token SUPERSHELL_START SUPERSHELL_END SUBSHELL_START SUBSHELL_END SUBSHELL_SUBSTITUTION_START SUBSHELL_SUBSTITUTION_END
%token ARRAY_ASSIGNMENT_START ARRAY_ASSIGNMENT_END
%token <int> DEPRECATED_SUBSHELL_START DEPRECATED_SUBSHELL_END
%token LPAREN RPAREN

%token KEYWORD_CLASS KEYWORD_VIRTUAL KEYWORD_METHOD KEYWORD_CONSTRUCTOR KEYWORD_DESTRUCTOR
%token KEYWORD_NEW KEYWORD_DELETE KEYWORD_NULLPTR

%token <bpp::AST::Token<std::string>> IDENTIFIER IDENTIFIER_LVALUE
%token KEYWORD_PUBLIC KEYWORD_PRIVATE KEYWORD_PROTECTED
%token KEYWORD_TYPEOF

%token ARRAY_INDEX_START ARRAY_INDEX_END LBRACKET RBRACKET
%token REF_START REF_START_LVALUE REF_END
%token <bpp::AST::Token<std::string>> BASH_VAR
%token BASH_VAR_START BASH_VAR_END
%token HASH
%token HEREDOC_CONTENT_START HERESTRING_START
%token <bpp::AST::Token<std::string>> HEREDOC_START HEREDOC_DELIMITER HEREDOC_END
%token BASH_KEYWORD_CASE BASH_KEYWORD_IN BASH_CASE_PATTERN_DELIM BASH_CASE_PATTERN_TERMINATOR BASH_KEYWORD_ESAC
%token <bpp::AST::Token<std::string>> BASH_CASE_BODY_BEGIN
%token BASH_KEYWORD_SELECT BASH_KEYWORD_FOR BASH_KEYWORD_DO BASH_KEYWORD_DONE
%token ARITH_FOR_CONDITION_START ARITH_FOR_CONDITION_END
%token INCREMENT_OPERATOR DECREMENT_OPERATOR
%token <bpp::AST::Token<std::string>> INTEGER COMPARISON_OPERATOR
%token BASH_KEYWORD_IF BASH_KEYWORD_THEN BASH_KEYWORD_ELIF BASH_KEYWORD_ELSE BASH_KEYWORD_FI
%token BASH_KEYWORD_WHILE BASH_KEYWORD_UNTIL
%token BASH_KEYWORD_FUNCTION BASH_FUNCTION_OPEN
%token <bpp::AST::Token<std::string>> BASH_FUNCTION_LABEL
%token BASH_KEYWORD_LOCAL

%token EXCLAM
%token <bpp::AST::Token<std::string>> EXPANSION_BEGIN PARAMETER_EXPANSION_CONTENT

%token <bpp::AST::Token<std::string>> PROCESS_SUBSTITUTION_START
%token PROCESS_SUBSTITUTION_END
%token BASH_ARITHMETIC_START BASH_ARITHMETIC_END
%token <bpp::AST::Token<std::string>> BASH_53_NATIVE_SUPERSHELL_START
%token BASH_53_NATIVE_SUPERSHELL_END

/* For detecting possible early-exit paths */
%token <bpp::AST::Token<std::string>> BASH_KEYWORD_RETURN BASH_KEYWORD_EXIT BASH_KEYWORD_EXEC BASH_KEYWORD_BREAK BASH_KEYWORD_CONTINUE

/* For named file descriptors as in {var}<>file */
%token <bpp::AST::Token<std::string>> BASH_NAMED_FD

/* Handling unrecognized tokens */
%token <bpp::AST::Token<std::string>> CATCHALL


%precedence CONCAT_STOP
%precedence LANGLE LANGLE_AMPERSAND RANGLE RANGLE_AMPERSAND AMPERSAND_RANGLE HEREDOC_START HERESTRING_START BASH_ARITHMETIC_START ARRAY_ASSIGNMENT_START BASH_53_NATIVE_SUPERSHELL_START
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

%type <bpp::AST::Token<bpp::AST::IncludeStatement::IncludeKeyword>> include_keyword
%type <ASTNodePtr> include_statement

%type <ASTNodePtr> class_definition
%type <bpp::AST::Token<bpp::AST::AccessModifier>> access_modifier access_modifier_keyword
%type <ASTNodePtr> datamember_declaration

%type <bpp::AST::Token<bpp::AST::MethodDefinition::Parameter>> parameter
%type <std::vector<bpp::AST::Token<bpp::AST::MethodDefinition::Parameter>>> maybe_parameter_list
%type <ASTNodePtr> method_definition constructor_definition destructor_definition

%type <ASTNodePtr> block supershell subshell_raw subshell_substitution dollar_subshell deprecated_subshell

%type <ASTNodePtr> object_instantiation instantiation_suffix
%type <ASTNodePtr> pointer_declaration pointer_declaration_preface
%type <ASTNodePtr> maybe_descend_object_hierarchy object_reference object_reference_lvalue self_reference self_reference_lvalue
%type <ASTNodePtr> object_address pointer_dereference pointer_dereference_rvalue pointer_dereference_lvalue

%type <ASTNodePtr> delete_statement new_statement

%type <ASTNodePtr> doublequoted_string quote_contents string_interpolation

%type <ASTNodePtr> valid_rvalue concatenatable_rvalue concatenated_rvalue
%type <std::vector<ASTNodePtr>> sequence_of_rvalues
%type <ASTNodePtr> value_assignment maybe_value_assignment
%type <ASTNodePtr> object_assignment shell_variable_assignment

%type <ASTNodePtr> typeof_expression
%type <ASTNodePtr> dynamic_cast cast_target
%type <ASTNodePtr> bash_variable maybe_array_index maybe_parameter_expansion array_index

%type <ASTNodePtr> process_substitution
%type <ASTNodePtr> heredoc_body heredoc_content herestring
%type <ASTNodePtr> redirection
%type <ASTNodePtr> operative_command_element simple_command_element simple_command simple_pipeline simple_command_sequence
%type <ASTNodePtr> pipeline shell_command_sequence shell_command

%type <ASTNodePtr> operative_command_word

%type <bpp::AST::Token<std::string>> raw_text_token

%type <std::vector<ASTNodePtr>> command_redirections

%type <ASTNodePtr> bash_if_statement bash_if_condition bash_if_root_branch bash_if_else_branch
%type <std::vector<ASTNodePtr>> maybe_bash_if_else_branches

%type <ASTNodePtr> bash_for_statement bash_select_statement bash_for_or_select_header
%type <ASTNodePtr> bash_for_or_select_maybe_in_something bash_for_or_select_input
%type <ASTNodePtr> bash_case_statement bash_case_header bash_case_input bash_case_pattern bash_case_pattern_header
%type <std::vector<ASTNodePtr>> bash_case_body
%type <ASTNodePtr> bash_arithmetic_for_statement arithmetic_for_condition arith_statement increment_decrement_expression arith_condition_term comparison_expression

%type <ASTNodePtr> bash_while_statement bash_until_statement bash_while_or_until_condition
%type <ASTNodePtr> bash_function bash_arithmetic_substitution
%type <ASTNodePtr> bash_53_native_supershell
%type <ASTNodePtr> array_assignment

%type <ASTNodePtr> bash_break_or_continue_command maybe_break_or_continue_argument

%type <bpp::AST::Token<std::string>> maybe_include_type maybe_as_clause maybe_parent_class
%type <bpp::AST::Token<std::string>> assignment_operator
%type <bpp::AST::Token<std::string>> maybe_exclam
%type <bpp::AST::Token<std::string>> maybe_hash
%type <bpp::AST::Token<std::string>> heredoc_header
%type <bpp::AST::Token<std::string>> bash_for_or_select_variable
%type <bpp::AST::Token<std::string>> arith_operator comparison_operator
%type <bpp::AST::Token<std::string>> redirection_operator
%type <bpp::AST::Token<std::string>> logical_connective

%%

program: statements {
		program = std::make_unique<bpp::AST::Program>();
		program->addChildren(std::move($1));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		program->setPosition(line_number, column_number);
		program->setEndPosition(@1.end.line, @1.end.column);
	}
	;

statements:
	/* empty */ { $$ = std::vector<std::unique_ptr<bpp::AST::ASTNode>>(); }
	| statements statement { $$ = std::move($1); if ($2) $$.emplace_back(std::move($2)); }
	;

statement:
	DELIM {
		set_incoming_token_can_be_lvalue(true, yyscanner);
		set_received_local_keyword(false, yyscanner);
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| shell_command_sequence %prec CONCAT_STOP { $$ = std::move($1); }
	| include_statement { $$ = std::move($1); }
	| class_definition { $$ = std::move($1); }
	| datamember_declaration { $$ = std::move($1); }
	| method_definition { $$ = std::move($1); }
	| constructor_definition { $$ = std::move($1); }
	| destructor_definition { $$ = std::move($1); }
	| object_instantiation { $$ = std::move($1); }
	| delete_statement { $$ = std::move($1); }
	| bash_function { $$ = std::move($1); }
	| error DELIM {
		set_incoming_token_can_be_lvalue(true, yyscanner);
		set_received_local_keyword(false, yyscanner);

		$$ = nullptr; // Don't create an AST node for this

		/**
		 * From GNU Bison 3.8.1 manual, Section 6 "Error Recovery":
		 * > To prevent an outpouring of error messages, the parser
		 * > will output no error message for another syntax error
		 * > that happens shortly after the first; only after three
		 * > consecutive input tokens have been successfully shifted
		 * > will error messages resume.
		 * > ...
		 * > You can make error messages resume immediately by using
		 * > the macro yyerrok in an action.
		 */
		yyerrok; // Allow error messages to resume immediately
	}
	;

shell_command_sequence:
	pipeline %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommandSequence>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| shell_command_sequence logical_connective maybe_whitespace pipeline {
		auto commandSequence = static_uniqueptr_cast<bpp::AST::BashCommandSequence>(std::move($1));
		auto connective = std::make_unique<bpp::AST::RawText>();
		connective->setText($2);
		commandSequence->addChild(std::move(connective));
		commandSequence->addChild(std::move($4));
		commandSequence->setEndPosition(@4.end.line, @4.end.column);
		$$ = std::move(commandSequence);
	}
	;

pipeline:
	shell_command %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashPipeline>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| pipeline PIPE maybe_whitespace shell_command {
		auto pipeline = static_uniqueptr_cast<bpp::AST::BashPipeline>(std::move($1));
		pipeline->addText(" | "); // Preserve pipe symbol
		pipeline->addChild(std::move($4));
		pipeline->setEndPosition(@4.end.line, @4.end.column);
		pipeline->unmarkAllExitPaths(); //'return', 'exit', and 'exec' can't exit the program here, since they are part of longer pipelines

		$$ = std::move(pipeline);
	}
	;

logical_connective:
	DOUBLEAMPERSAND { $$ = " && "; }
	| DOUBLEPIPE { $$ = " || "; }
	;

shell_command:
	simple_command %prec CONCAT_STOP { current_command_can_receive_lvalues = true; $$ = std::move($1); }
	| bash_case_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| bash_select_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| bash_for_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| bash_arithmetic_for_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| bash_if_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| bash_while_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| bash_until_statement command_redirections %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	| shell_command heredoc_body {
		$1->addChild(std::move($2));
		$1->setEndPosition(@2.end.line, @2.end.column);
		$$ = std::move($1);
	}
	;

command_redirections:
	/* empty */ %prec CONCAT_STOP { $$ = std::vector<ASTNodePtr>(); }
	| command_redirections redirection { $$ = std::move($1); $$.emplace_back(std::move($2)); }
	| command_redirections WS redirection { $$ = std::move($1); $$.emplace_back(std::move($3)); }
	;

simple_command_sequence:
	simple_pipeline %prec CONCAT_STOP {
		auto node = std::make_unique<bpp::AST::BashCommandSequence>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| simple_command_sequence logical_connective maybe_whitespace simple_pipeline {
		auto commandSequence = static_uniqueptr_cast<bpp::AST::BashCommandSequence>(std::move($1));
		auto connective = std::make_unique<bpp::AST::RawText>();
		connective->setText($2);
		commandSequence->addChild(std::move(connective));
		commandSequence->addChild(std::move($4));
		commandSequence->setEndPosition(@4.end.line, @4.end.column);
		$$ = std::move(commandSequence);
	}
	;

simple_pipeline:
	simple_command %prec CONCAT_STOP {
		current_command_can_receive_lvalues = true;
		auto node = std::make_unique<bpp::AST::BashPipeline>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| simple_pipeline PIPE maybe_whitespace simple_command {
		current_command_can_receive_lvalues = true;

		auto pipeline = static_uniqueptr_cast<bpp::AST::BashPipeline>(std::move($1));
		pipeline->addText(" | "); // Preserve pipe symbol
		pipeline->addChild(std::move($4));
		pipeline->setEndPosition(@4.end.line, @4.end.column);
		pipeline->unmarkAllExitPaths(); //'return', 'exit', and 'exec' can't exit program/function here, since they are part of longer pipelines
		// Likewise, 'break' and 'continue' can't exit any loops here, for the same reason
		$$ = std::move(pipeline);
	}
	;

simple_command:
	simple_command_element {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| BASH_KEYWORD_RETURN {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		auto rawTextNode = std::make_unique<bpp::AST::RawText>();
		rawTextNode->setPosition(line_number, column_number);
		rawTextNode->setEndPosition(@1.end.line, @1.end.column);
		rawTextNode->setText($1);
		node->addChild(std::move(rawTextNode));
		node->setExitPointType(bpp::ExitPointType::FUNCTION_EXIT);
		$$ = std::move(node);
	}
	| BASH_KEYWORD_EXIT {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		auto rawTextNode = std::make_unique<bpp::AST::RawText>();
		rawTextNode->setPosition(line_number, column_number);
		rawTextNode->setEndPosition(@1.end.line, @1.end.column);
		rawTextNode->setText($1);
		node->addChild(std::move(rawTextNode));
		node->setExitPointType(bpp::ExitPointType::PROGRAM_EXIT);
		$$ = std::move(node);
	}
	| BASH_KEYWORD_EXEC {
		auto node = std::make_unique<bpp::AST::BashCommand>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		auto rawTextNode = std::make_unique<bpp::AST::RawText>();
		rawTextNode->setPosition(line_number, column_number);
		rawTextNode->setEndPosition(@1.end.line, @1.end.column);
		rawTextNode->setText($1);
		node->addChild(std::move(rawTextNode));
		node->setIsExec(true); // exec *might* exit early, depends on later parsing
		// Exec only exits early if a non-option argument that is not encased in curly-braces is provided.
		// E.g., exec program-name, or exec -l program-name
		// exec {var}<>file will not exit. 'exec' on its own will not exit. 'exec -l' (no program given) will not exit.
		// Likewise, exec -a ID will not exit. '-a' is the only option to exec that takes an argument.
		// The argument also needs to not be an argument to -a.
		exec_received_A_option = false;
		$$ = std::move(node);
	}
	| bash_break_or_continue_command { $$ = std::move($1); }
	| simple_command WS simple_command_element {
		auto command = static_uniqueptr_cast<bpp::AST::BashCommand>(std::move($1));
		auto* rawElement = $3.get();
		command->addText(" "); // Preserve whitespace
		command->addChild(std::move($3));
		command->setEndPosition(@3.end.line, @3.end.column);

		// Below is a massive HACK to deal with the fact that 'exec' only exits early under certain conditions
		// TODO(@rail5): Brittle, dependent on specific AST structure
		// If:
		// 1. The command is 'exec'
		// 2. The next element is an rvalue (not a redirection, etc)
		if (command->isExec() && rawElement->getType() == bpp::AST::NodeType::Rvalue) {
			// 3. The rvalue actually has a child node
			if (auto* r_child = rawElement->getFirstChild()) {
				// 4. The rvalue child is a RawText node
				// 5. The option is not the argument to '-a'
				// 6. The RawText node's text does not start with a hyphen
				if (r_child->getType() == bpp::AST::NodeType::RawText) {
					auto* rawTextNode = static_cast<bpp::AST::RawText*>(r_child);
					if (exec_received_A_option) {
						// If the -a option was received, then the next argument is the ID to use for the exec'd program.
						// This is not an early exit point, so we don't setIsEarlyExitPoint(true)
						exec_received_A_option = false; // Reset for next command
					} else if (rawTextNode->TEXT().getValue() == "-a") {
						// If the -a option is received, then the next argument is the ID to use for the exec'd program.
						// This is not an early exit point, so we don't setIsEarlyExitPoint(true)
						exec_received_A_option = true; // Set for next command
					} else if (!rawTextNode->TEXT().getValue().starts_with('-')) {
						command->setExitPointType(bpp::ExitPointType::PROGRAM_EXIT);
					}
				}
			}
		}
		$$ = std::move(command);
	}
	| simple_command redirection {
		$1->addChild(std::move($2));
		$1->setEndPosition(@2.end.line, @2.end.column);
		$$ = std::move($1);
	}
	;

maybe_break_or_continue_argument:
	/* empty */ { $$ = nullptr; }
	| WS valid_rvalue { $$ = std::move($2); }
	;

bash_break_or_continue_command:
	BASH_KEYWORD_BREAK maybe_break_or_continue_argument {
		// This business with 'maybe_argument' introduces a shift/reduce conflict,
		// but the default action of shifting is exactly what we want.
		auto node = std::make_unique<bpp::AST::BashBreakOrContinueCommand>();
		node->setPosition(@1.begin.line, @1.begin.column);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setIsBreak(true);
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	| BASH_KEYWORD_CONTINUE maybe_break_or_continue_argument {
		auto node = std::make_unique<bpp::AST::BashBreakOrContinueCommand>();
		node->setPosition(@1.begin.line, @1.begin.column);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setIsBreak(false);
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	;

simple_command_element:
	shell_variable_assignment { $$ = std::move($1); }
	| object_assignment { $$ = std::move($1); }
	| pointer_declaration { $$ = std::move($1); }
	| redirection { $$ = std::move($1); }
	| BASH_NAMED_FD {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| operative_command_element { current_command_can_receive_lvalues = false; $$ = std::move($1); }
	| valid_rvalue %prec CONCAT_STOP { current_command_can_receive_lvalues = false; $$ = std::move($1); }
	| block { current_command_can_receive_lvalues = false; $$ = std::move($1); }
	;

operative_command_element:
	operative_command_word { $$ = std::move($1); }
	| object_reference_lvalue { $$ = std::move($1); }
	| self_reference_lvalue { $$ = std::move($1); }
	| pointer_dereference_lvalue { $$ = std::move($1); }
	;

/*
 * In Bash syntax, a single "word" can be composed of multiple adjacent tokens with no whitespace.
 * We already support this for rvalues via concatenated_rvalue, but the first word of a command
 * is frequently lexed as IDENTIFIER_LVALUE (because that information is important for later parsing).
 *
 * Without this rule, `command-with-hyphens` is forced to reduce `command` as a complete command word,
 * and then `-with-hyphens` is parsed as a second command.
 *
 *
 * TODO(@rail5): Consider the possibility of re-working the grammar for recursive lexing/parsing of "WORD" tokens, the way Bash does it
 *
 * Bash, for example, when it sees `var=value`, sees a single ASSIGNMENT_WORD token,
 * Which it later expands by again "parsing" the internal *contents* of that token.
 *   (not using the same parser, but "expanding" nonetheless to figure out what the lvalue is, what the rvalue is, etc.)
 *
 * This means that the Bash lexer/parser does an indeterminate number of passes over source,
 *   (indeterminate if you don't know the source beforehand, that is)
 * possibly expanding further and further with each inner layer/command substitution/etc.
 *   (e.g., if there are 5 nested subshells, that's 5 passes, each time lexing a new "single token" that then needs to be parsed again)
 *
 * By contrast, our lexer/parser does only a *single* pass over the source,
 * lexing every component separately and relying on the *parser* to understand how they fit together.
 * This is more traditional, but maybe less "true to Bash" in spirit.
 *
 * In the above example, where Bash sees `var=value` => ASSIGNMENT_WORD,
 * we instead see `var=value` => IDENTIFIER_LVALUE `=` IDENTIFIER,
 * and our parser then understands that this is an assignment *statement*.
 *
 * Our approach is arguably simpler and arguably more efficient since we avoid multiple passes over the source,
 * but it also necessitates that the lexer retain extra state about the current parsing context,
 * and more than this: it's a goddamned rat-race to make sure we're parsing Bash correctly.
 *
 * The single greatest advantage I can think of to "doing it the way Bash does it"
 * would be that we could *know* that we're parsing Bash correctly,
 * without always feeling like we're running on this treadmill to get the Bash parts of the language right.
 */
operative_command_word:
	IDENTIFIER_LVALUE {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| operative_command_word raw_text_token {
		auto node = static_uniqueptr_cast<bpp::AST::RawText>(std::move($1));
		node->appendText($2.getValue());
		node->setEndPosition(@2.end.line, @2.end.column);
		$$ = std::move(node);
	}
	;

raw_text_token:
	IDENTIFIER { $$ = $1; }
	| INTEGER { $$ = $1; }
	| SINGLEQUOTED_STRING { $$ = $1; }
	| CATCHALL { $$ = $1; }
	| KEYWORD_NULLPTR { $$ = bpp::AST::Token<std::string>("0", @1.begin.line, @1.begin.column); }
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

		// NOTE on why "if (current_command_can_receive_lvalues)" is necessary here:
		// The lexer's lookahead token may have been a token which signals the start of a NEW command,
		// E.g., a pipe, logical connective, or delimiter
		// In that case, the lexer will have set the "can be lvalue" flag to true in anticipation of the next command,
		// and here we'd accidentally end up overriding that to false!
		// (LALR(1) is no joke)
		// So, refuse to *ever* set it to false if the lexer disagrees,
		// And only ever tell the lexer "hey, you missed a spot" -- never "hey, you got a spot wrong"

		if (current_command_can_receive_lvalues)
			set_incoming_token_can_be_lvalue(true, yyscanner);

		auto node = std::make_unique<bpp::AST::BashRedirection>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setOperator($1);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}
	| heredoc_header {
		if (current_command_can_receive_lvalues)
			set_incoming_token_can_be_lvalue(true, yyscanner);
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| herestring {
		if (current_command_can_receive_lvalues)
			set_incoming_token_can_be_lvalue(true, yyscanner);
		$$ = std::move($1);
	}
	;

redirection_operator:
	LANGLE { $$ = $1; }
	| LANGLE RANGLE { $$ = bpp::AST::Token<std::string>($1.getValue() + $2.getValue(), @1.begin.line, @1.begin.column); }
	| LANGLE_AMPERSAND { $$ = $1; }
	| RANGLE { $$ = $1; }
	| RANGLE RANGLE { $$ = bpp::AST::Token<std::string>($1.getValue() + $2.getValue(), @1.begin.line, @1.begin.column); }
	| RANGLE_AMPERSAND { $$ = $1; }
	| RANGLE PIPE { $$ = bpp::AST::Token<std::string>($1.getValue() + "|", @1.begin.line, @1.begin.column); }
	| AMPERSAND_RANGLE { $$ = $1; }
	| AMPERSAND_RANGLE RANGLE { $$ = bpp::AST::Token<std::string>($1.getValue() + $2.getValue(), @1.begin.line, @1.begin.column); }
	;

block:
	LBRACE whitespace_or_delimiter statements RBRACE {
		auto node = std::make_unique<bpp::AST::Block>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@4.end.line, @4.end.column);
		node->addChildren(std::move($3));
		$$ = std::move(node);
	}
	;

valid_rvalue:
	EMPTY_ASSIGNMENT {
		auto node = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(line_number, column_number); // EMPTY_ASSIGNMENT is a zero-length token
		node->addText("");
		$$ = std::move(node);
	}
	| array_assignment {
		auto node = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| subshell_raw {
		auto node = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| new_statement {
		auto node = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| dynamic_cast {
		auto node = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| typeof_expression {
		auto node = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| concatenated_rvalue %prec CONCAT_STOP { $$ = std::move($1); }
	;

array_assignment:
	ARRAY_ASSIGNMENT_START statements ARRAY_ASSIGNMENT_END {
		auto node = std::make_unique<bpp::AST::ArrayAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	;

concatenated_rvalue:
	concatenatable_rvalue %prec CONCAT_STOP {
		auto rvalue = std::make_unique<bpp::AST::Rvalue>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		rvalue->setPosition(line_number, column_number);
		rvalue->setEndPosition(@1.end.line, @1.end.column);
		rvalue->addChild(std::move($1));
		$$ = std::move(rvalue);
	}
	| concatenated_rvalue concatenatable_rvalue {
		auto rvalue = static_uniqueptr_cast<bpp::AST::Rvalue>(std::move($1));
		rvalue->addChild(std::move($2));
		rvalue->setEndPosition(@2.end.line, @2.end.column);
		$$ = std::move(rvalue);
	}
	;

concatenatable_rvalue:
	IDENTIFIER {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| INTEGER {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| SINGLEQUOTED_STRING {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| KEYWORD_NULLPTR {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText(bpp::AST::Token<std::string>("0", line_number, column_number)); // Represent nullptr as 0
		$$ = std::move(node);
	}
	| CATCHALL { 
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| doublequoted_string { $$ = std::move($1); }
	| object_reference { $$ = std::move($1); }
	| self_reference { $$ = std::move($1); }
	| object_address { $$ = std::move($1); }
	| pointer_dereference_rvalue { $$ = std::move($1); }
	| bash_variable { $$ = std::move($1); }
	| supershell { $$ = std::move($1); }
	| subshell_substitution { $$ = std::move($1); }
	| process_substitution { $$ = std::move($1); }
	| bash_arithmetic_substitution { $$ = std::move($1); }
	| bash_53_native_supershell { $$ = std::move($1); }
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
		bpp::AST::Token<bpp::AST::IncludeStatement::IncludeKeyword> keyword = $1;
		bpp::AST::Token<bpp::AST::IncludeStatement::IncludeType> type;
		type.setLine(@2.begin.line);
		type.setCharPositionInLine(@2.begin.column);
		if ($2 == "dynamic") {
			type.setValue(bpp::AST::IncludeStatement::IncludeType::DYNAMIC);
		} else {
			type.setValue(bpp::AST::IncludeStatement::IncludeType::STATIC);
		}

		bpp::AST::IncludeStatement::PathType pathType;
		std::string pathText = $3;
		if (pathText.front() == '<') {
			pathType = bpp::AST::IncludeStatement::PathType::ANGLEBRACKET;
		} else {
			pathType = bpp::AST::IncludeStatement::PathType::QUOTED;
		}

		bpp::AST::Token<std::string> path(pathText, @3.begin.line, @3.begin.column);

		std::string asPathText = $4;
		std::uint32_t asPathLine = $4.getLine();
		std::uint32_t asPathColumn = $4.getCharPositionInLine();
		bpp::AST::Token<std::string> asPath(asPathText, asPathLine, asPathColumn);

		auto node = std::make_unique<bpp::AST::IncludeStatement>();

		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@5.end.line, @5.end.column);
		node->setKeyword(keyword);
		node->setType(type);
		node->setPathType(pathType);
		node->setPath(path);
		node->setAsPath(asPath);

		$$ = std::move(node);
	}
	;

include_keyword:
	KEYWORD_INCLUDE { $$ = bpp::AST::IncludeStatement::IncludeKeyword::INCLUDE; }
	| KEYWORD_INCLUDE_ALWAYS { $$ = bpp::AST::IncludeStatement::IncludeKeyword::INCLUDE_ALWAYS; }
	;

maybe_include_type:
	WS { $$ = ""; }
	| WS INCLUDE_TYPE WS { $$ = $2; }
	;

maybe_as_clause:
	maybe_whitespace { $$ = ""; }
	| WS KEYWORD_AS WS INCLUDE_PATH maybe_whitespace { $$ = $4; }
	;

object_instantiation:
	AT_LVALUE IDENTIFIER instantiation_suffix {
		if ($3 == nullptr) {
			// Not an object instantiation, but an lvalue object reference
			auto node = std::make_unique<bpp::AST::ObjectReference>();
			std::uint32_t line_number = @1.begin.line;
			std::uint32_t column_number = @1.begin.column;
			node->setPosition(line_number, column_number);
			node->setEndPosition(@2.end.line, @2.end.column);
			node->setRootIdentifier($2);
			node->setLvalue(true);
			node->setAddressOf(false);
			node->setPointerDereference(false);
			node->setSelfReference(false);

			$$ = std::move(node);
		} else {
			// Use the ObjectInstantiation node returned by instantiation_suffix
			auto node = static_uniqueptr_cast<bpp::AST::ObjectInstantiation>(std::move($3));
			std::uint32_t line_number = @1.begin.line;
			std::uint32_t column_number = @1.begin.column;
			node->setPosition(line_number, column_number);
			node->setEndPosition(@3.end.line, @3.end.column);
			node->setType($2);
			$$ = std::move(node);
		}
	}
	;

instantiation_suffix:
	WS IDENTIFIER maybe_value_assignment {
		auto node = std::make_unique<bpp::AST::ObjectInstantiation>();
		node->setIdentifier($2);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}
	| WS { $$ = nullptr; }
	;

pointer_declaration:
	pointer_declaration_preface WS IDENTIFIER_LVALUE maybe_value_assignment {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectInstantiation>(std::move($1));
		node->setIdentifier($3);
		node->setEndPosition(@4.end.line, @4.end.column);
		node->addChild(std::move($4));

		$$ = std::move(node);
	}
	;

pointer_declaration_preface:
	AT_LVALUE IDENTIFIER ASTERISK {
		set_incoming_token_can_be_lvalue(true, yyscanner); // The following identifier should be an lvalue, let the lexer know

		auto node = std::make_unique<bpp::AST::ObjectInstantiation>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		node->setType($2);
		node->setIsPointer(true);

		$$ = std::move(node);
	}

new_statement:
	KEYWORD_NEW WS IDENTIFIER {
		auto node = std::make_unique<bpp::AST::NewStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setType($3);

		$$ = std::move(node);
	}
	;

delete_statement:
	KEYWORD_DELETE WS object_reference {
		auto node = std::make_unique<bpp::AST::DeleteStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChild(std::move($3));

		$$ = std::move(node);
	}
	|
	KEYWORD_DELETE WS self_reference {
		auto node = std::make_unique<bpp::AST::DeleteStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChild(std::move($3));

		$$ = std::move(node);
	}
	;

class_definition:
	KEYWORD_CLASS WS IDENTIFIER maybe_parent_class block {
		auto node = std::make_unique<bpp::AST::ClassDefinition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@5.end.line, @5.end.column);
		node->setClassName($3);
		node->setParentClassName($4);
		node->addChild(std::move($5));
		$$ = std::move(node);
	}
	;

maybe_parent_class:
	whitespace_or_delimiter { $$ = ""; }
	| WS COLON WS IDENTIFIER whitespace_or_delimiter { $$ = $4; }
	;

datamember_declaration:
	access_modifier IDENTIFIER_LVALUE maybe_value_assignment maybe_whitespace DELIM {
		auto node = std::make_unique<bpp::AST::DatamemberDeclaration>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setAccessModifier($1);
		node->setIdentifier($2);
		node->addChild(std::move($3));

		$$ = std::move(node);
	}
	| access_modifier object_instantiation maybe_whitespace DELIM {
		auto node = std::make_unique<bpp::AST::DatamemberDeclaration>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		if (dynamic_cast<bpp::AST::ObjectInstantiation*>($2.get()) == nullptr) {
			// Special-case: `@public @id`
			// where `@id` has been given to us by ObjectInstantiation
			// ObjectInstantiation should NOT capture that as one of its alternatives,
			// but DOES (presently) in order to handle an ambiguity between object instantiations and object references in other contexts
			// However:
			// In the event that an ObjectInstantiation DOES capture `@id` alone (instead of `@id id`),
			// it does not return it as a pointer to bpp::AST::ObjectInstantiation, but instead returns it
			// as a pointer to bpp::AST::ObjectReference with the lvalue flag set.
			// At the very least, this allows us to detect this case.
			// This whole thing reeks of one massive HACK however, from the way we chose to resolve the ambiguity to handling it here.
			// TODO(@rail5): Clean this up

			// Error out and stop parsing
			error(@2, "Invalid datamember declaration");
			YYERROR;
		}

		node->setAccessModifier($1);
		node->addChild(std::move($2));

		$$ = std::move(node);
	}
	| access_modifier pointer_declaration maybe_whitespace DELIM {
		auto node = std::make_unique<bpp::AST::DatamemberDeclaration>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setAccessModifier($1);
		node->addChild(std::move($2));

		$$ = std::move(node);
	}
	;

access_modifier:
	access_modifier_keyword WS {
		$$ = $1;
	}
	;

access_modifier_keyword:
	KEYWORD_PUBLIC { $$ = bpp::AST::AccessModifier::PUBLIC; }
	| KEYWORD_PRIVATE { $$ = bpp::AST::AccessModifier::PRIVATE; }
	| KEYWORD_PROTECTED { $$ = bpp::AST::AccessModifier::PROTECTED; }
	;

maybe_value_assignment:
	/* empty */ { $$ = nullptr; }
	| value_assignment { $$ = std::move($1); }
	;

value_assignment:
	assignment_operator valid_rvalue {
		auto node = std::make_unique<bpp::AST::ValueAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setOperator($1);
		node->addChild(std::move($2));

		$$ = std::move(node);
	}
	;

assignment_operator:
	EQUALS {
		set_parsed_assignment_operator(true, yyscanner);
		$$ = "=";
	}
	| PLUS_EQUALS {
		set_parsed_assignment_operator(true, yyscanner);
		$$ = "+=";
	}
	;

method_definition:
	access_modifier KEYWORD_METHOD WS IDENTIFIER WS maybe_parameter_list block {
		auto node = std::make_unique<bpp::AST::MethodDefinition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@7.end.line, @7.end.column);

		node->setAccessModifier($1);
		node->setName($4);
		node->addParameters($6);
		node->addChild(std::move($7));

		node->setVirtual(false);

		$$ = std::move(node);
	}
	| KEYWORD_VIRTUAL WS access_modifier KEYWORD_METHOD WS IDENTIFIER WS maybe_parameter_list block {
		auto node = std::make_unique<bpp::AST::MethodDefinition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@9.end.line, @9.end.column);

		node->setAccessModifier($3);
		node->setName($6);
		node->addParameters($8);
		node->addChild(std::move($9));

		node->setVirtual(true);

		$$ = std::move(node);
	}
	;

maybe_parameter_list:
	/* empty */ { $$ = std::vector<bpp::AST::Token<bpp::AST::MethodDefinition::Parameter>>(); }
	| maybe_parameter_list parameter { $$ = std::move($1); $$.push_back($2); }
	;

parameter:
	IDENTIFIER WS {
		bpp::AST::MethodDefinition::Parameter param;
		param.type = std::nullopt;
		param.name = $1;
		param.pointer = false;
		bpp::AST::Token<bpp::AST::MethodDefinition::Parameter> token;
		token.setValue(param);
		token.setLine(@1.begin.line);
		token.setCharPositionInLine(@1.begin.column);
		$$ = std::move(token);
	}
	| AT IDENTIFIER ASTERISK WS IDENTIFIER WS {
		bpp::AST::MethodDefinition::Parameter param;
		param.type = $2;
		param.name = $5;
		param.pointer = true;
		bpp::AST::Token<bpp::AST::MethodDefinition::Parameter> token;
		token.setValue(param);
		token.setLine(@1.begin.line);
		token.setCharPositionInLine(@1.begin.column);
		$$ = std::move(token);
	}
	| AT IDENTIFIER WS IDENTIFIER WS {
		/* Actually invalid, but error handling should come later when traversing the AST */
		/* Invalid because methods cannot take non-primitive parameters */
		bpp::AST::MethodDefinition::Parameter param;
		param.type = $2;
		param.name = $4;
		param.pointer = false;
		bpp::AST::Token<bpp::AST::MethodDefinition::Parameter> token;
		token.setValue(param);
		token.setLine(@1.begin.line);
		token.setCharPositionInLine(@1.begin.column);
		$$ = std::move(token);
	}
	;

constructor_definition:
	KEYWORD_CONSTRUCTOR WS block {
		auto node = std::make_unique<bpp::AST::ConstructorDefinition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}
	;

destructor_definition:
	KEYWORD_DESTRUCTOR WS block {
		auto node = std::make_unique<bpp::AST::DestructorDefinition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}
	;

doublequoted_string:
	QUOTE_BEGIN quote_contents QUOTE_END {
		auto node = static_uniqueptr_cast<bpp::AST::DoublequotedString>(std::move($2));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);

		$$ = std::move(node);
	}
	;

quote_contents:
	/* empty */ { $$ = std::make_unique<bpp::AST::DoublequotedString>(); }
	| quote_contents STRING_CONTENT {
		$$ = std::move($1);
		$$->setEndPosition(@2.end.line, @2.end.column);
		static_cast<bpp::AST::DoublequotedString*>($$.get())->addText($2);
	}
	| quote_contents string_interpolation {
		$$ = std::move($1);
		$$->setEndPosition(@2.end.line, @2.end.column);
		static_cast<bpp::AST::DoublequotedString*>($$.get())->addChild(std::move($2));
	}
	;

string_interpolation:
	object_reference { $$ = std::move($1); }
	| self_reference { $$ = std::move($1); }
	| object_address { $$ = std::move($1); }
	| pointer_dereference { $$ = std::move($1); }
	| supershell { $$ = std::move($1); }
	| subshell_substitution { $$ = std::move($1); }
	| bash_arithmetic_substitution { $$ = std::move($1); }
	| bash_53_native_supershell { $$ = std::move($1); }
	| bash_variable { $$ = std::move($1); }
	;

object_reference:
	AT IDENTIFIER maybe_descend_object_hierarchy {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($3));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);

		node->setRootIdentifier($2);
		node->setLvalue(false);
		node->setSelfReference(false);

		$$ = std::move(node);
	}
	| REF_START maybe_hash IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($4));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);

		node->setRootIdentifier($3);
		node->setLvalue(false);
		node->setSelfReference(false);

		if (!$2.getValue().empty()) {
			node->setHasHashkey(true);
		}

		node->addChild(std::move($5));

		$$ = std::move(node);
	}
	;

object_reference_lvalue:
	AT_LVALUE IDENTIFIER maybe_descend_object_hierarchy maybe_array_index {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($3));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);

		node->setRootIdentifier($2);
		node->setLvalue(true);
		node->setSelfReference(false);

		node->addChild(std::move($4));

		$$ = std::move(node);
	}
	| REF_START_LVALUE maybe_hash IDENTIFIER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($4));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);

		node->setRootIdentifier($3);
		node->setLvalue(true);
		node->setSelfReference(false);

		if (!$2.getValue().empty()) {
			node->setHasHashkey(true);
		}

		node->addChild(std::move($5));

		$$ = std::move(node);
	}
	;

self_reference:
	KEYWORD_THIS maybe_descend_object_hierarchy {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("this", @1.begin.line, @1.begin.column + 1));
		node->setLvalue(false);
		node->setSelfReference(true);

		$$ = std::move(node);
	}
	| REF_START maybe_hash KEYWORD_THIS maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($4));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("this", @3.begin.line, @3.begin.column + 1));
		node->setLvalue(false);
		node->setSelfReference(true);

		if (!$2.getValue().empty()) {
			node->setHasHashkey(true);
		}

		node->addChild(std::move($5));

		$$ = std::move(node);
	}
	| KEYWORD_SUPER maybe_descend_object_hierarchy {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("super", @1.begin.line, @1.begin.column + 1));
		node->setLvalue(false);
		node->setSelfReference(true);

		$$ = std::move(node);
	}
	| REF_START maybe_hash KEYWORD_SUPER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($4));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("super", @3.begin.line, @3.begin.column + 1));
		node->setLvalue(false);
		node->setSelfReference(true);

		if (!$2.getValue().empty()) {
			node->setHasHashkey(true);
		}

		node->addChild(std::move($5));

		$$ = std::move(node);
	}
	;

self_reference_lvalue:
	KEYWORD_THIS_LVALUE maybe_descend_object_hierarchy maybe_array_index {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("this", @1.begin.line, @1.begin.column + 1));
		node->setLvalue(true);
		node->setSelfReference(true);

		node->addChild(std::move($3));

		$$ = std::move(node);
	}
	| REF_START_LVALUE maybe_hash KEYWORD_THIS maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($4));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("this", @3.begin.line, @3.begin.column + 1));
		node->setLvalue(true);
		node->setSelfReference(true);

		if (!$2.getValue().empty()) {
			node->setHasHashkey(true);
		}

		node->addChild(std::move($5));

		$$ = std::move(node);
	}
	| KEYWORD_SUPER_LVALUE maybe_descend_object_hierarchy {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("super", @1.begin.line, @1.begin.column + 1));
		node->setLvalue(true);
		node->setSelfReference(true);

		$$ = std::move(node);
	}
	| REF_START_LVALUE maybe_hash KEYWORD_SUPER maybe_descend_object_hierarchy maybe_array_index REF_END {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($4));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);

		node->setRootIdentifier(bpp::AST::Token<std::string>("super", @3.begin.line, @3.begin.column + 1));
		node->setLvalue(true);
		node->setSelfReference(true);

		if (!$2.getValue().empty()) {
			node->setHasHashkey(true);
		}

		node->addChild(std::move($5));

		$$ = std::move(node);
	}
	;

maybe_descend_object_hierarchy:
	/* empty */ { $$ = std::make_unique<bpp::AST::ObjectReference>(); }
	| maybe_descend_object_hierarchy DOT IDENTIFIER {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($1));
		node->addIdentifier($3);
		$$ = std::move(node);
	}
	;

maybe_array_index:
	/* empty */ { $$ = nullptr; }
	| ARRAY_INDEX_START array_index ARRAY_INDEX_END {
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		$2->setPosition(line_number, column_number);
		$2->setEndPosition(@3.end.line, @3.end.column);
		$$ = std::move($2);
	}
	;

array_index:
	valid_rvalue {
		auto node = std::make_unique<bpp::AST::ArrayIndex>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| AT {
		// '@' is a valid array index, as in ${array[@]}
		auto node = std::make_unique<bpp::AST::ArrayIndex>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		auto atNode = std::make_unique<bpp::AST::RawText>();
		atNode->setPosition(line_number, column_number);
		atNode->setText(bpp::AST::Token<std::string>("@", line_number, column_number));
		node->addChild(std::move(atNode));
		$$ = std::move(node);
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
		auto node = std::make_unique<bpp::AST::BashVariable>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@7.end.line, @7.end.column);

		bpp::AST::Token<std::string> text;
		text.setLine(@2.begin.line);
		text.setCharPositionInLine(@2.begin.column);
		text.setValue($2.getValue() + $3.getValue() + $4.getValue());
		node->setText(text);
		node->addChild(std::move($5));
		node->addChild(std::move($6));
		$$ = std::move(node);
	}
	| BASH_VAR {
		auto node = std::make_unique<bpp::AST::BashVariable>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);

		bpp::AST::Token<std::string> text;
		text.setLine(line_number);
		// Skip the '$' at the beginning of the token
		text.setCharPositionInLine(column_number + 1);
		text.setValue($1.getValue().substr(1));
		node->setText(text);
		$$ = std::move(node);
	}
	;

maybe_parameter_expansion:
	/* empty */ { $$ = nullptr; }
	| EXPANSION_BEGIN valid_rvalue {
		auto node = std::make_unique<bpp::AST::ParameterExpansion>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setExpansionBegin($1);
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	| EXPANSION_BEGIN PARAMETER_EXPANSION_CONTENT {
		auto node = std::make_unique<bpp::AST::ParameterExpansion>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setExpansionBegin($1);
		auto contentNode = std::make_unique<bpp::AST::RawText>();
		contentNode->setPosition(@2.begin.line, @2.begin.column);
		contentNode->setText($2);
		node->addChild(std::move(contentNode));
		$$ = std::move(node);
	}
	;

dynamic_cast:
	KEYWORD_DYNAMIC_CAST LANGLE cast_target RANGLE WS valid_rvalue {
		auto node = std::make_unique<bpp::AST::DynamicCast>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);
		node->addChild(std::move($3)); // cast_target
		node->addChild(std::move($6)); // valid_rvalue
		$$ = std::move(node);
	}
	;

cast_target:
	IDENTIFIER {
		auto node = std::make_unique<bpp::AST::DynamicCastTarget>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setTargetType($1);
		$$ = std::move(node);
	}
	| bash_variable {
		auto node = std::make_unique<bpp::AST::DynamicCastTarget>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| object_reference {
		auto node = std::make_unique<bpp::AST::DynamicCastTarget>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| self_reference {
		auto node = std::make_unique<bpp::AST::DynamicCastTarget>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	;

object_assignment:
	object_reference_lvalue value_assignment {
		set_incoming_token_can_be_lvalue(true, yyscanner); // Lvalues can follow assignments

		auto node = std::make_unique<bpp::AST::ObjectAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	| self_reference_lvalue value_assignment {
		set_incoming_token_can_be_lvalue(true, yyscanner); // Lvalues can follow assignments

		auto node = std::make_unique<bpp::AST::ObjectAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	| pointer_dereference_lvalue value_assignment {
		set_incoming_token_can_be_lvalue(true, yyscanner); // Lvalues can follow assignments

		auto node = std::make_unique<bpp::AST::ObjectAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	;

shell_variable_assignment:
	IDENTIFIER_LVALUE value_assignment {
		set_incoming_token_can_be_lvalue(true, yyscanner); // Lvalues can follow assignments

		auto node = std::make_unique<bpp::AST::PrimitiveAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setIdentifier($1);
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	| BASH_KEYWORD_LOCAL WS IDENTIFIER_LVALUE maybe_value_assignment {
		set_incoming_token_can_be_lvalue(true, yyscanner); // Lvalues can follow assignments
		set_received_local_keyword(true, yyscanner); // Mark that we received the 'local' keyword for this line

		auto node = std::make_unique<bpp::AST::PrimitiveAssignment>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@4.end.line, @4.end.column);
		node->setLocal(true);
		node->setIdentifier($3);
		if ($4 != nullptr) node->addChild(std::move($4));
		$$ = std::move(node);
	}
	;

object_address:
	AMPERSAND object_reference {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '&' token
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setAddressOf(true);

		$$ = std::move(node);
	}
	| AMPERSAND self_reference {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '&' token
		node->setEndPosition(@2.end.line, @2.end.column);

		node->setAddressOf(true);

		$$ = std::move(node);
	}
	;

pointer_dereference:
	pointer_dereference_rvalue { $$ = std::move($1); }
	| pointer_dereference_lvalue { $$ = std::move($1); }
	;

pointer_dereference_rvalue:
	DEREFERENCE_OPERATOR object_reference {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setPointerDereference(true);
		$$ = std::move(node);
	}
	| DEREFERENCE_OPERATOR self_reference {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setPointerDereference(true);
		$$ = std::move(node);
	}
	;

pointer_dereference_lvalue:
	DEREFERENCE_OPERATOR object_reference_lvalue {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setPointerDereference(true);
		$$ = std::move(node);
	}
	| DEREFERENCE_OPERATOR self_reference_lvalue {
		auto node = static_uniqueptr_cast<bpp::AST::ObjectReference>(std::move($2));
		node->setPosition(@1.begin.line, @1.begin.column); // Move start position to '*' token
		node->setEndPosition(@2.end.line, @2.end.column);
		node->setPointerDereference(true);
		$$ = std::move(node);
	}
	;

typeof_expression:
	KEYWORD_TYPEOF WS valid_rvalue {
		auto node = std::make_unique<bpp::AST::TypeofExpression>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}

supershell:
	SUPERSHELL_START statements SUPERSHELL_END {
		auto node = std::make_unique<bpp::AST::Supershell>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	;

subshell_raw:
	SUBSHELL_START statements SUBSHELL_END {
		auto node = std::make_unique<bpp::AST::RawSubshell>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	;

subshell_substitution:
	dollar_subshell { $$ = std::move($1); }
	| deprecated_subshell { $$ = std::move($1); }
	;

dollar_subshell:
	SUBSHELL_SUBSTITUTION_START statements SUBSHELL_SUBSTITUTION_END {
		auto node = std::make_unique<bpp::AST::SubshellSubstitution>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		// Special check, barely documented by Bash (https://www.gnu.org/software/bash/manual/bash.html#Command-Substitution):
		// If 'statements' contains ONLY an input redirection and NOTHING ELSE,
		// Then this is not in fact a subshell substitution, but a replacement for the 'cat' command.
		if (is_only_input_redirection($2)) {
			node->setIsCatReplacement(true);
		}
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	;

deprecated_subshell:
	DEPRECATED_SUBSHELL_START statements DEPRECATED_SUBSHELL_END {
		// NOTE: The nesting depth is stored as the semantic value of the DEPRECATED_SUBSHELL_START token
		assert($1 == $3 && "Mismatched deprecated subshell nesting depths!");

		auto node = std::make_unique<bpp::AST::SubshellSubstitution>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	;

process_substitution:
	PROCESS_SUBSTITUTION_START statements PROCESS_SUBSTITUTION_END {
		auto node = std::make_unique<bpp::AST::ProcessSubstitution>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setSubstitutionStart($1);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}
	;

heredoc_header:
	HEREDOC_START HEREDOC_DELIMITER {
		bpp::AST::Token<std::string> header;
		header.setLine(@1.begin.line);
		header.setCharPositionInLine(@1.begin.column);
		header.setValue($1.getValue() + $2.getValue());
		$$ = std::move(header);
	}
	;

heredoc_body:
	HEREDOC_CONTENT_START heredoc_content HEREDOC_END {
		auto node = static_uniqueptr_cast<bpp::AST::HeredocBody>(std::move($2));
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setDelimiter($3);
		$$ = std::move(node);
	}
	;

heredoc_content:
	/* empty */ { $$ = std::make_unique<bpp::AST::HeredocBody>(); }
	| heredoc_content STRING_CONTENT {
		auto node = static_uniqueptr_cast<bpp::AST::HeredocBody>(std::move($1));
		node->addText($2);
		$$ = std::move(node);
	}
	| heredoc_content string_interpolation {
		auto node = static_uniqueptr_cast<bpp::AST::HeredocBody>(std::move($1));
		node->addChild(std::move($2));
		$$ = std::move(node);
		}
	;

herestring:
	HERESTRING_START maybe_whitespace valid_rvalue {
		auto node = std::make_unique<bpp::AST::HereString>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}
	;

bash_case_statement:
	BASH_KEYWORD_CASE WS bash_case_header bash_case_body BASH_KEYWORD_ESAC {
		auto node = std::make_unique<bpp::AST::BashCaseStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@5.end.line, @5.end.column);
		node->addChild(std::move($3)); // bash_case_input
		node->addChildren(std::move($4)); // bash_case_body
		$$ = std::move(node);
	}
	;

bash_case_header:
	bash_case_input WS BASH_KEYWORD_IN BASH_CASE_BODY_BEGIN { $$ = std::move($1); }
	;

bash_case_input:
	valid_rvalue {
		set_bash_case_input_received(true, yyscanner);
		auto node = std::make_unique<bpp::AST::BashCaseInput>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	;

bash_case_body:
	/* empty */ { $$ = std::vector<ASTNodePtr>(); }
	| bash_case_body bash_case_pattern { $$ = std::move($1); $$.emplace_back(std::move($2)); }
	;

bash_case_pattern:
	bash_case_pattern_header BASH_CASE_PATTERN_DELIM statements BASH_CASE_PATTERN_TERMINATOR {
		auto node = std::make_unique<bpp::AST::BashCasePattern>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@4.end.line, @4.end.column);
		node->addChild(std::move($1)); // pattern header
		node->addChildren(std::move($3)); // statements
		$$ = std::move(node);
	}
	;

bash_case_pattern_header:
	/* empty */ { $$ = std::make_unique<bpp::AST::BashCasePatternHeader>(); }
	| bash_case_pattern_header STRING_CONTENT {
		auto node = static_uniqueptr_cast<bpp::AST::BashCasePatternHeader>(std::move($1));
		node->setPosition(@1.begin.line, @1.begin.column);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addText($2);
		$$ = std::move(node);
	}
	| bash_case_pattern_header string_interpolation {
		auto node = static_uniqueptr_cast<bpp::AST::BashCasePatternHeader>(std::move($1));
		node->setPosition(@1.begin.line, @1.begin.column);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($2));
		$$ = std::move(node);
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
		auto* forStatement = dynamic_cast<bpp::AST::BashForStatement*>($3.get());
		std::unique_ptr<bpp::AST::BashSelectStatement> selectStatement;
		if (!forStatement) {
			// This should not happen, but just in case
			selectStatement = static_uniqueptr_cast<bpp::AST::BashSelectStatement>(std::move($3));
		} else {
			selectStatement = std::make_unique<bpp::AST::BashSelectStatement>();
			selectStatement->setVariable(forStatement->VARIABLE());
			selectStatement->addChildren(forStatement->releaseChildren());
		}
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		selectStatement->setPosition(line_number, column_number);
		selectStatement->setEndPosition(@8.end.line, @8.end.column);

		selectStatement->addChildren(std::move($7));
		$$ = std::move(selectStatement);
	}
	| BASH_KEYWORD_SELECT WS bash_for_or_select_header DELIM maybe_whitespace block {
		auto* forStatement = dynamic_cast<bpp::AST::BashForStatement*>($3.get());
		std::unique_ptr<bpp::AST::BashSelectStatement> selectStatement;
		if (!forStatement) {
			// This should not happen, but just in case
			selectStatement = static_uniqueptr_cast<bpp::AST::BashSelectStatement>(std::move($3));
		} else {
			selectStatement = std::make_unique<bpp::AST::BashSelectStatement>();
			selectStatement->setVariable(forStatement->VARIABLE());
			selectStatement->addChildren(forStatement->releaseChildren());
		}
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		selectStatement->setPosition(line_number, column_number);
		selectStatement->setEndPosition(@6.end.line, @6.end.column);

		selectStatement->addChild(std::move($6));
		$$ = std::move(selectStatement);
	}
	;

bash_for_or_select_header:
	bash_for_or_select_variable bash_for_or_select_maybe_in_something {
		// Here, we assume that it's a 'for' statement by default
		// 'for' is much more common than 'select'
		// If it winds up being 'select' instead, the AST node will be updated later
		// When we're in the bash_select_statement rule
		auto forStatement = std::make_unique<bpp::AST::BashForStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		forStatement->setPosition(line_number, column_number);
		forStatement->setVariable($1);
		forStatement->addChild(std::move($2));
		$$ = std::move(forStatement);
	}
	;

bash_for_or_select_maybe_in_something:
	maybe_whitespace { $$ = nullptr; }
	| WS BASH_KEYWORD_IN WS bash_for_or_select_input {
		auto inCondition = static_uniqueptr_cast<bpp::AST::BashInCondition>(std::move($4));
		inCondition->setPosition(@2.begin.line, @2.begin.column); // Move position to 'in' token
		inCondition->setEndPosition(@4.end.line, @4.end.column);
		$$ = std::move(inCondition);
	}
	| WS BASH_KEYWORD_IN maybe_whitespace {
		auto node = std::make_unique<bpp::AST::BashInCondition>();
		std::uint32_t line_number = @2.begin.line;
		std::uint32_t column_number = @2.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		$$ = std::move(node); // 'in' with no input, valid in Bash
	}
	;

bash_for_or_select_variable:
	IDENTIFIER {
		set_bash_for_or_select_variable_received(true, yyscanner);
		$$ = $1;
	}
	;

bash_for_or_select_input:
	valid_rvalue {
		auto node = std::make_unique<bpp::AST::BashInCondition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| bash_for_or_select_input WS valid_rvalue {
		auto inCondition = static_uniqueptr_cast<bpp::AST::BashInCondition>(std::move($1));
		inCondition->addText(" "); // Preserve whitespace between items
		inCondition->addChild(std::move($3));
		inCondition->setEndPosition(@3.end.line, @3.end.column);
		$$ = std::move(inCondition);
	}
	| bash_for_or_select_input WS { $$ = std::move($1); } /* Allow trailing whitespace */
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
		std::unique_ptr<bpp::AST::BashForStatement> forStatement;
		auto* previous_forStatement = dynamic_cast<bpp::AST::BashForStatement*>($3.get());
		if (!previous_forStatement) {
			// This should not happen, but just in case
			auto selectStatement = static_uniqueptr_cast<bpp::AST::BashSelectStatement>(std::move($3));
			forStatement = std::make_unique<bpp::AST::BashForStatement>();
			forStatement->setVariable(selectStatement->VARIABLE());
			forStatement->addChildren(selectStatement->releaseChildren());
		} else {
			forStatement = static_uniqueptr_cast<bpp::AST::BashForStatement>(std::move($3));
		}
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		forStatement->setPosition(line_number, column_number);
		forStatement->setEndPosition(@8.end.line, @8.end.column);
		forStatement->addChildren(std::move($7));
		$$ = std::move(forStatement);
	}
	| BASH_KEYWORD_FOR WS bash_for_or_select_header DELIM maybe_whitespace block {
		std::unique_ptr<bpp::AST::BashForStatement> forStatement;
		auto* previous_forStatement = dynamic_cast<bpp::AST::BashForStatement*>($3.get());
		if (!previous_forStatement) {
			// This should not happen, but just in case
			auto selectStatement = static_uniqueptr_cast<bpp::AST::BashSelectStatement>(std::move($3));
			forStatement = std::make_unique<bpp::AST::BashForStatement>();
			forStatement->setVariable(selectStatement->VARIABLE());
			forStatement->addChildren(selectStatement->releaseChildren());
		} else {
			forStatement = static_uniqueptr_cast<bpp::AST::BashForStatement>(std::move($3));
		}
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		forStatement->setPosition(line_number, column_number);
		forStatement->setEndPosition(@6.end.line, @6.end.column);
		forStatement->addChild(std::move($6));
		$$ = std::move(forStatement);
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
		auto node = std::make_unique<bpp::AST::BashArithmeticForStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);
		node->addChild(std::move($3)); // for condition
		node->addChildren(std::move($5)); // statements
		$$ = std::move(node);
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition DELIM maybe_whitespace BASH_KEYWORD_DO statements BASH_KEYWORD_DONE {
		auto node = std::make_unique<bpp::AST::BashArithmeticForStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@8.end.line, @8.end.column);
		node->addChild(std::move($3)); // for condition
		node->addChildren(std::move($7)); // statements
		$$ = std::move(node);
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition block {
		auto node = std::make_unique<bpp::AST::BashArithmeticForStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@4.end.line, @4.end.column);
		node->addChild(std::move($3)); // for condition
		node->addChild(std::move($4)); // block
		$$ = std::move(node);
	}
	| BASH_KEYWORD_FOR WS arithmetic_for_condition DELIM maybe_whitespace block {
		auto node = std::make_unique<bpp::AST::BashArithmeticForStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@6.end.line, @6.end.column);
		node->addChild(std::move($3)); // for condition
		node->addChild(std::move($6)); // block
		$$ = std::move(node);
	}
	;

arithmetic_for_condition:
	ARITH_FOR_CONDITION_START arith_statement DELIM arith_statement DELIM arith_statement ARITH_FOR_CONDITION_END maybe_whitespace {
		auto node = std::make_unique<bpp::AST::BashArithmeticForCondition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@7.end.line, @7.end.column);
		node->addChild(std::move($2)); // first expression
		node->addText(" ; ");    // first delimiter
		node->addChild(std::move($4)); // second expression
		node->addText(" ; ");    // second delimiter
		node->addChild(std::move($6)); // third expression
		$$ = std::move(node);
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
	/* empty */ { $$ = std::make_unique<bpp::AST::BashArithmeticStatement>(); }
	| valid_rvalue {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| IDENTIFIER_LVALUE {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		auto rawText = std::make_unique<bpp::AST::RawText>();
		rawText->setPosition(line_number, column_number);
		rawText->setEndPosition(@1.end.line, @1.end.column);
		rawText->setText($1);
		node->addChild(std::move(rawText));
		$$ = std::move(node);
	}
	| object_reference_lvalue {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| self_reference_lvalue {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| object_assignment {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| shell_variable_assignment {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	| increment_decrement_expression { $$ = std::move($1); }
	| comparison_expression { $$ = std::move($1); }
	;

increment_decrement_expression:
	arith_condition_term arith_operator {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addChild(std::move($1));
		node->addText($2);
		$$ = std::move(node);
	}
	| arith_operator arith_condition_term {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@2.end.line, @2.end.column);
		node->addText($1);
		node->addChild(std::move($2));
		$$ = std::move(node);
	}
	;

comparison_expression:
	arith_condition_term maybe_whitespace comparison_operator maybe_whitespace arith_condition_term {
		auto node = std::make_unique<bpp::AST::BashArithmeticStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@5.end.line, @5.end.column);
		node->addChild(std::move($1));
		node->addText($3);
		node->addChild(std::move($5));
		$$ = std::move(node);
	}
	;

comparison_operator:
	COMPARISON_OPERATOR { $$ = $1; }
	| LANGLE { $$ = "<"; }
	| RANGLE { $$ = ">"; }
	;

arith_condition_term:
	object_reference { $$ = std::move($1); }
	| object_reference_lvalue { $$ = std::move($1); }
	| self_reference { $$ = std::move($1);}
	| self_reference_lvalue {$$ = std::move($1); }
	| bash_variable { $$ = std::move($1); }
	| IDENTIFIER_LVALUE {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| IDENTIFIER {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| INTEGER {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText($1);
		$$ = std::move(node);
	}
	| KEYWORD_NULLPTR {
		auto node = std::make_unique<bpp::AST::RawText>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->setText(bpp::AST::Token<std::string>("0", line_number, column_number));
		$$ = std::move(node);
	}
	;

arith_operator:
	INCREMENT_OPERATOR { $$ = "++"; }
	| DECREMENT_OPERATOR { $$ = "--"; }
	;

sequence_of_rvalues:
	valid_rvalue { $$ = std::vector<ASTNodePtr>(); $$.emplace_back(std::move($1)); }
	| sequence_of_rvalues WS valid_rvalue {
		auto node = std::move($1);
		node.emplace_back(std::move($3));
		$$ = std::move(node);
	}
	;

bash_arithmetic_substitution:
	BASH_ARITHMETIC_START sequence_of_rvalues BASH_ARITHMETIC_END {
		auto node = std::make_unique<bpp::AST::BashArithmeticSubstitution>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}

bash_if_statement:
	bash_if_root_branch maybe_bash_if_else_branches BASH_KEYWORD_FI {
		auto node = static_uniqueptr_cast<bpp::AST::BashIfStatement>(std::move($1));
		node->setEndPosition(@3.end.line, @3.end.column);
		node->addChildren(std::move($2)); // elif / else branches
		$$ = std::move(node);
	}
	;

bash_if_root_branch:
	BASH_KEYWORD_IF bash_if_condition DELIM maybe_whitespace BASH_KEYWORD_THEN maybe_whitespace statements {
		auto node = std::make_unique<bpp::AST::BashIfStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);

		auto rootBranch = std::make_unique<bpp::AST::BashIfBranch>();
		rootBranch->setPosition(@1.begin.line, @1.begin.column);
		rootBranch->setEndPosition(@7.end.line, @7.end.column);
		rootBranch->setHasCondition(true);
		rootBranch->setIsRootBranch(true);
		rootBranch->addChild(std::move($2)); // condition
		rootBranch->addChildren(std::move($7)); // statements

		node->addChild(std::move(rootBranch));
		$$ = std::move(node);
	}
	;

bash_if_condition:
	simple_command_sequence {
		set_bash_if_condition_received(true, yyscanner);
		auto node = std::make_unique<bpp::AST::BashIfCondition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	;

maybe_bash_if_else_branches:
	/* empty */ { $$ = std::vector<ASTNodePtr>(); }
	| maybe_bash_if_else_branches bash_if_else_branch { $$ = std::move($1); $$.emplace_back(std::move($2)); }
	;

bash_if_else_branch:
	BASH_KEYWORD_ELIF bash_if_condition DELIM maybe_whitespace BASH_KEYWORD_THEN maybe_whitespace statements {
		auto node = std::make_unique<bpp::AST::BashIfBranch>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@7.end.line, @7.end.column);
		node->setHasCondition(true);
		node->setIsRootBranch(false);
		node->addChild(std::move($2)); // condition
		node->addChildren(std::move($7)); // statements

		$$ = std::move(node);
	}
	| BASH_KEYWORD_ELSE DELIM maybe_whitespace statements {
		auto node = std::make_unique<bpp::AST::BashIfBranch>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@4.end.line, @4.end.column);
		// 'else' branch has no condition
		node->setHasCondition(false);
		node->setIsRootBranch(false);
		node->addChildren(std::move($4)); // statements

		$$ = std::move(node);
	}
	;

bash_while_statement:
	BASH_KEYWORD_WHILE bash_while_or_until_condition DELIM maybe_whitespace BASH_KEYWORD_DO maybe_whitespace statements BASH_KEYWORD_DONE {
		auto node = std::make_unique<bpp::AST::BashWhileStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@8.end.line, @8.end.column);
		node->addChild(std::move($2)); // condition
		node->addChildren(std::move($7)); // statements
		$$ = std::move(node);
	}
	;

bash_until_statement:
	BASH_KEYWORD_UNTIL bash_while_or_until_condition DELIM maybe_whitespace BASH_KEYWORD_DO maybe_whitespace statements BASH_KEYWORD_DONE {
		auto node = std::make_unique<bpp::AST::BashUntilStatement>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@8.end.line, @8.end.column);
		node->addChild(std::move($2)); // condition
		node->addChildren(std::move($7)); // statements
		$$ = std::move(node);
	}
	;

bash_while_or_until_condition:
	simple_command_sequence {
		set_bash_while_or_until_condition_received(true, yyscanner);
		auto node = std::make_unique<bpp::AST::BashWhileOrUntilCondition>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@1.end.line, @1.end.column);
		node->addChild(std::move($1));
		$$ = std::move(node);
	}
	;
/*
 * Valid declaration forms for functions as parsed by Bash:
 * 1. function name { ... }
 * 2. function name() { ... }
 * 3. name() { ... }
 */
bash_function:
	BASH_KEYWORD_FUNCTION BASH_FUNCTION_LABEL block {
		auto node = std::make_unique<bpp::AST::BashFunction>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setName($2);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}
	| BASH_KEYWORD_FUNCTION BASH_FUNCTION_LABEL BASH_FUNCTION_OPEN block {
		auto node = std::make_unique<bpp::AST::BashFunction>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@4.end.line, @4.end.column);
		node->setName($2);
		node->addChild(std::move($4));
		$$ = std::move(node);
	}
	| BASH_FUNCTION_LABEL BASH_FUNCTION_OPEN block {
		auto node = std::make_unique<bpp::AST::BashFunction>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setName($1);
		node->addChild(std::move($3));
		$$ = std::move(node);
	}

bash_53_native_supershell:
	BASH_53_NATIVE_SUPERSHELL_START statements BASH_53_NATIVE_SUPERSHELL_END {
		auto node = std::make_unique<bpp::AST::Bash53NativeSupershell>();
		std::uint32_t line_number = @1.begin.line;
		std::uint32_t column_number = @1.begin.column;
		node->setPosition(line_number, column_number);
		node->setEndPosition(@3.end.line, @3.end.column);
		node->setStartToken($1);
		node->addChildren(std::move($2));
		$$ = std::move(node);
	}

%%

namespace yy {
void parser::error(const location_type& loc, const std::string& m) {
	const std::uint32_t line = loc.begin.line;
	const std::uint32_t column = loc.begin.column;
	const std::uint32_t length =
		loc.begin.line == loc.end.line
			? loc.end.column - loc.begin.column
			: UINT32_MAX;
	bpp::ErrorHandling::ParserError error(
		include_chain,
		line,
		column,
		length,
		m,
		lsp_mode
	);
	errors.push_back(error);
}
} // namespace yy
