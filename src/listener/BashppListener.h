/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_BASHPPLISTENER_H_
#define SRC_LISTENER_BASHPPLISTENER_H_

#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <memory>
#include <stack>
#include <antlr4-runtime.h>

#include "../antlr/BashppParserBaseListener.h"

#include "../bpp_include/bpp_entity.cpp"
#include "../bpp_include/bpp_code_entity.cpp"
#include "../bpp_include/bpp_string.cpp"
#include "../bpp_include/bpp_value_assignment.cpp"
#include "../bpp_include/bpp_object_reference.cpp"
#include "../bpp_include/bpp_object_assignment.cpp"
#include "../bpp_include/bpp_program.cpp"
#include "../bpp_include/bpp_class.cpp"
#include "../bpp_include/bpp_constructor.cpp"
#include "../bpp_include/bpp_destructor.cpp"
#include "../bpp_include/bpp_datamember.cpp"
#include "../bpp_include/bpp_method.cpp"
#include "../bpp_include/bpp_method_parameter.cpp"
#include "../bpp_include/bpp_object.cpp"

#include "../syntax_error.cpp"
#include "../internal_error.cpp"

#define skip_comment if (in_comment) return;
#define skip_singlequote_string if (in_singlequote_string) return;
#define skip_syntax_errors if (error_thrown) { \
		if (error_context == ctx) { \
			error_thrown = false; \
			error_context = nullptr; \
		} \
		return; \
		}

class BashppListener : public BashppParserBaseListener {
	private:
		std::string source_file;
		std::ostream* output_stream = &std::cout;
		std::string output_file;
		bool run_on_exit = false;

		std::shared_ptr<bpp::bpp_program> program = std::make_shared<bpp::bpp_program>();

		bool in_comment = false;
		bool in_singlequote_string = false;

		std::stack<std::shared_ptr<bpp::bpp_entity>> entity_stack;
		// The entity_stack is used to keep track of the current entity being processed

		std::set<std::string> protected_keywords = {
			"class", "constructor", "delete", "destructor",
			"include", "include_once", "method", "new",
			"nullptr", "primitive", "private", "protected",
			"public", "this", "virtual"
		};

		std::shared_ptr<bpp::bpp_class> primitive;

		uint64_t supershell_counter = 0;
		uint64_t new_counter = 0;

		struct code_segment {
			std::string pre_code;
			std::string code;
			std::string post_code;
		};

		code_segment generate_supershell_code(std::string code_to_run_in_supershell) {
			code_segment result;

			std::string supershell_function_name = "____supershellRunFunc" + std::to_string(supershell_counter);
			std::string supershell_output_variable = "____supershellOutput" + std::to_string(supershell_counter);

			result.pre_code += "function " + supershell_function_name + "() {\n";
			result.pre_code += "	" + code_to_run_in_supershell + "\n";
			result.pre_code += "}\n";
			result.pre_code += "bpp____supershell " + supershell_output_variable + " " + supershell_function_name + "\n";

			result.post_code += "unset -f " + supershell_function_name + "\n";
			result.post_code += "unset " + supershell_output_variable + "\n";

			result.code = "${" + supershell_output_variable + "}";

			supershell_counter++;

			return result;
		}

		code_segment generate_new_code(std::shared_ptr<bpp::bpp_class> object_class) {
			code_segment result;

			std::string new_function_name = "bpp__" + object_class->get_name() + "____new";
			std::string new_output_variable = "____newOutput" + std::to_string(new_counter);

			result.pre_code += new_function_name + " \"\" " + new_output_variable + "\n";
			result.code = "${" + new_output_variable + "}";
			result.post_code += "unset " + new_output_variable + "\n";

			new_counter++;

			return result;
		}

		bool error_thrown = false;
		antlr4::ParserRuleContext* error_context = nullptr;

		bool program_has_errors = false;

		#define set_error_context error_thrown = true; error_context = ctx;

		#define output_syntax_error(symbol, msg) \
			int line = symbol->getLine(); \
			int column = symbol->getCharPositionInLine(); \
			print_syntax_error(source_file, line, column, msg); \
			program_has_errors = true;

		#define throw_syntax_error_sym(symbol, msg) \
			output_syntax_error(symbol, msg) \
			set_error_context \
			return;

		#define throw_syntax_error(token, msg) antlr4::Token* symbol = token->getSymbol(); \
			throw_syntax_error_sym(symbol, msg) \
			set_error_context \
			return;
		
		#define throw_syntax_error_from_exitRule(token, msg) antlr4::Token* symbol = token->getSymbol(); \
			output_syntax_error(symbol, msg) \
			return;
		
	public:

	void set_source_file(std::string source_file) {
		this->source_file = source_file;
	}

	void set_output_stream(std::ostream* output_stream) {
		this->output_stream = output_stream;
	}

	void set_output_file(std::string output_file) {
		this->output_file = output_file;
	}

	void set_run_on_exit(bool run_on_exit) {
		this->run_on_exit = run_on_exit;
	}

	void enterProgram(BashppParser::ProgramContext *ctx) override;
	void exitProgram(BashppParser::ProgramContext *ctx) override;

	void enterInclude_statement(BashppParser::Include_statementContext *ctx) override;
	void exitInclude_statement(BashppParser::Include_statementContext *ctx) override;

	void enterClass_definition(BashppParser::Class_definitionContext *ctx) override;
	void exitClass_definition(BashppParser::Class_definitionContext *ctx) override;

	void enterMember_declaration(BashppParser::Member_declarationContext *ctx) override;
	void exitMember_declaration(BashppParser::Member_declarationContext *ctx) override;

	void enterObject_instantiation(BashppParser::Object_instantiationContext *ctx) override;
	void exitObject_instantiation(BashppParser::Object_instantiationContext *ctx) override;

	void enterPointer_declaration(BashppParser::Pointer_declarationContext *ctx) override;
	void exitPointer_declaration(BashppParser::Pointer_declarationContext *ctx) override;

	void enterValue_assignment(BashppParser::Value_assignmentContext *ctx) override;
	void exitValue_assignment(BashppParser::Value_assignmentContext *ctx) override;

	void enterMethod_definition(BashppParser::Method_definitionContext *ctx) override;
	void exitMethod_definition(BashppParser::Method_definitionContext *ctx) override;

	void enterConstructor_definition(BashppParser::Constructor_definitionContext *ctx) override;
	void exitConstructor_definition(BashppParser::Constructor_definitionContext *ctx) override;

	void enterDestructor_definition(BashppParser::Destructor_definitionContext *ctx) override;
	void exitDestructor_definition(BashppParser::Destructor_definitionContext *ctx) override;

	void enterSelf_reference(BashppParser::Self_referenceContext *ctx) override;
	void exitSelf_reference(BashppParser::Self_referenceContext *ctx) override;

	void enterSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) override;
	void exitSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) override;

	void enterStatement(BashppParser::StatementContext *ctx) override;
	void exitStatement(BashppParser::StatementContext *ctx) override;

	void enterClass_body_statement(BashppParser::Class_body_statementContext *ctx) override;
	void exitClass_body_statement(BashppParser::Class_body_statementContext *ctx) override;

	void enterGeneral_statement(BashppParser::General_statementContext *ctx) override;
	void exitGeneral_statement(BashppParser::General_statementContext *ctx) override;

	void enterObject_assignment(BashppParser::Object_assignmentContext *ctx) override;

	void exitObject_assignment(BashppParser::Object_assignmentContext *ctx) override;

	void enterPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) override;
	void exitPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) override;

	void enterObject_address(BashppParser::Object_addressContext *ctx) override;
	void exitObject_address(BashppParser::Object_addressContext *ctx) override;

	void enterObject_reference(BashppParser::Object_referenceContext *ctx) override;
	void exitObject_reference(BashppParser::Object_referenceContext *ctx) override;

	void enterObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) override;
	void exitObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) override;

	void enterNullptr_ref(BashppParser::Nullptr_refContext *ctx) override;
	void exitNullptr_ref(BashppParser::Nullptr_refContext *ctx) override;

	void enterNew_statement(BashppParser::New_statementContext *ctx) override;
	void exitNew_statement(BashppParser::New_statementContext *ctx) override;

	void enterDelete_statement(BashppParser::Delete_statementContext *ctx) override;
	void exitDelete_statement(BashppParser::Delete_statementContext *ctx) override;

	void enterSupershell(BashppParser::SupershellContext *ctx) override;
	void exitSupershell(BashppParser::SupershellContext *ctx) override;

	void enterSubshell(BashppParser::SubshellContext *ctx) override;
	void exitSubshell(BashppParser::SubshellContext *ctx) override;

	void enterDeprecated_subshell(BashppParser::Deprecated_subshellContext *ctx) override;
	void exitDeprecated_subshell(BashppParser::Deprecated_subshellContext *ctx) override;

	void enterBash_arithmetic(BashppParser::Bash_arithmeticContext *ctx) override;
	void exitBash_arithmetic(BashppParser::Bash_arithmeticContext *ctx) override;

	void enterString(BashppParser::StringContext *ctx) override;
	void exitString(BashppParser::StringContext *ctx) override;

	void enterSinglequote_string(BashppParser::Singlequote_stringContext *ctx) override;
	void exitSinglequote_string(BashppParser::Singlequote_stringContext *ctx) override;

	void enterComment(BashppParser::CommentContext *ctx) override;
	void exitComment(BashppParser::CommentContext *ctx) override;

	void enterParameter(BashppParser::ParameterContext *ctx) override;
	void exitParameter(BashppParser::ParameterContext *ctx) override;

	void enterOther_statement(BashppParser::Other_statementContext *ctx) override;
	void exitOther_statement(BashppParser::Other_statementContext *ctx) override;

	void enterRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) override;
	void exitRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) override;

	void enterExtra_statement(BashppParser::Extra_statementContext *ctx) override;
	void exitExtra_statement(BashppParser::Extra_statementContext *ctx) override;

};

#endif // SRC_LISTENER_BASHPPLISTENER_H_