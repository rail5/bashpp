/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BASHPPLISTENER_CPP_
#define ANTLR_BASHPPLISTENER_CPP_

#include <iostream>
#include <vector>
#include <antlr4-runtime.h>

#include "out/BashppParserBaseListener.h"

#include "bpp_include/bpp_program.cpp"
#include "bpp_include/bpp_class.cpp"
#include "bpp_include/bpp_constructor.cpp"
#include "bpp_include/bpp_destructor.cpp"
#include "bpp_include/bpp_datamember.cpp"
#include "bpp_include/bpp_method.cpp"
#include "bpp_include/bpp_method_parameter.cpp"
#include "bpp_include/bpp_object.cpp"

#define skip_comment if (in_comment) return;

class BashppListener : public BashppParserBaseListener {
	private:
		bpp::bpp_program program;

		bpp::bpp_class current_class;
		bpp::bpp_method current_method;

		bool in_comment = false;
		std::string generated_code = "#!/usr/bin/env bash\n";
		bool in_class = false;
		bool in_member_declaration = false;
		bool in_method_definition = false;

		std::string current_class_name = "";
		std::string current_member_name = "";
		std::string current_member_type = "";
		bpp::bpp_scope current_member_visibility = bpp::SCOPE_PRIVATE;
		std::string current_member_default_value = "";
		bool current_member_is_pointer = false;

		std::string current_method_name = "";
		std::string current_method_body = "";
		bpp::bpp_scope current_method_visibility = bpp::SCOPE_PRIVATE;
		bool current_method_is_virtual = false;
		std::vector<bpp::bpp_method_parameter> current_method_parameters;
	public:

	void enterProgram(BashppParser::ProgramContext *ctx) override {
		std::cout << "Entered Program" << std::endl;

		program.add_code(bpp_supershell_function);
	}
	void exitProgram(BashppParser::ProgramContext *ctx) override {
		std::cout << "Exited Program" << std::endl;

		std::cout << program.get_code() << std::endl;
	}

	void enterStatement(BashppParser::StatementContext *ctx) override { 
		skip_comment
	}
	void exitStatement(BashppParser::StatementContext *ctx) override { 
		skip_comment
	}

	void enterClass_body_statement(BashppParser::Class_body_statementContext *ctx) override { 
		skip_comment
	}
	void exitClass_body_statement(BashppParser::Class_body_statementContext *ctx) override { 
		skip_comment
	}

	void enterGeneral_statement(BashppParser::General_statementContext *ctx) override { 
		skip_comment
	}
	void exitGeneral_statement(BashppParser::General_statementContext *ctx) override { 
		skip_comment
	}

	void enterInclude_statement(BashppParser::Include_statementContext *ctx) override { 
		skip_comment
	}
	void exitInclude_statement(BashppParser::Include_statementContext *ctx) override { 
		skip_comment
	}

	void enterClass_definition(BashppParser::Class_definitionContext *ctx) override {
		skip_comment

		if (in_class) {
			std::cerr << "Error: Nested classes are not supported" << std::endl;
			exit(1);
		}
		std::cout << "Entered Class Definition" << std::endl;
		in_class = true;

		current_class.destroy();

		// Get class name
		current_class_name = ctx->IDENTIFIER()[0]->getText();
		std::cout << "Class name: " << current_class_name << std::endl;

		if (program.get_class(current_class_name) != nullptr) {
			std::cerr << "Error: Class '" << current_class_name << "' already exists" << std::endl;
			exit(1);
		}

		current_class.set_name(current_class_name);
		
		// Is this a derived class?
		if (ctx->IDENTIFIER().size() > 1) {
			std::string parent_class_name = ctx->IDENTIFIER()[1]->getText();
			std::cout << "Parent class name: " << parent_class_name << std::endl;

			if (program.get_class(parent_class_name) == nullptr) {
				std::cerr << "Error: Parent class '" << parent_class_name << "' does not exist" << std::endl;
				exit(1);
			}
			
			current_class.inherit(*program.get_class(parent_class_name));

		}

	}
	void exitClass_definition(BashppParser::Class_definitionContext *ctx) override {
		skip_comment

		program.add_class(current_class);

		std::cout << "Exited Class Definition" << std::endl;
		in_class = false;
		current_class_name.clear();
	}

	void enterMember_declaration(BashppParser::Member_declarationContext *ctx) override {
		skip_comment

		if (!in_class) {
			std::cerr << "Error: Member declaration outside of class" << std::endl;
			exit(1);
		}
		std::cout << "Entered Member Declaration for class " << current_class_name << std::endl;
		in_member_declaration = true;

		// Get member name & type
		// If it's a primitive, it will be a single IDENTIFIER
		// If it's an object, the IDENTIFIER will not be set, but the object_instantiation will be
		// If it's a pointer, neither IDENTIFIER nor object_instantiation will be set, but pointer_declaration will be
		// We'll catch objects & pointers in their respective rule contexts
		antlr4::tree::TerminalNode *identifier = ctx->IDENTIFIER();
		if (identifier != nullptr) {
			current_member_name = identifier->getText();
			current_member_type = "primitive";
		}

		// Get member visibility
		// One of KEYWORD_PUBLIC, KEYWORD_PRIVATE, KEYWORD_PROTECTED will be set
		if (ctx->KEYWORD_PUBLIC() != nullptr) {
			current_member_visibility = bpp::SCOPE_PUBLIC;
		} else if (ctx->KEYWORD_PRIVATE() != nullptr) {
			current_member_visibility = bpp::SCOPE_PRIVATE;
		} else if (ctx->KEYWORD_PROTECTED() != nullptr) {
			current_member_visibility = bpp::SCOPE_PROTECTED;
		}

		// Get default value (if any)
		if (ctx->value_assignment() != nullptr) {
			current_member_default_value = ctx->value_assignment()->acceptable_rvalue()->getText();
		}

	}
	void exitMember_declaration(BashppParser::Member_declarationContext *ctx) override {
		skip_comment

		bpp::bpp_datamember member(current_member_name);
		member.set_type(current_member_type);
		member.set_default_value(current_member_default_value);
		member.set_scope(current_member_visibility);

		current_class.add_datamember(member);

		std::cout << "Exited Member Declaration" << std::endl;
		in_member_declaration = false;

		std::cout << "Class '" << current_class_name << "' has a " << current_member_visibility << " member of type '" << current_member_type << "' named '" << current_member_name << "'" << std::endl;
		std::cout << "This member has the default value: " << current_member_default_value << std::endl;
		std::cout << "This member " << (current_member_is_pointer ? "is" : "is not") << " a pointer" << std::endl;
	}

	void enterMethod_definition(BashppParser::Method_definitionContext *ctx) override {
		skip_comment

		if (!in_class) {
			std::cerr << "Error: Method definition outside of class" << std::endl;
			exit(1);
		}
		std::cout << "Entered Method Definition" << std::endl;
		in_method_definition = true;

		current_method.destroy();

		// Get method name
		current_method_name = ctx->IDENTIFIER()->getText();

		// Get method visibility
		// One of KEYWORD_PUBLIC, KEYWORD_PRIVATE, KEYWORD_PROTECTED will be set
		if (ctx->KEYWORD_PUBLIC() != nullptr) {
			current_method_visibility = bpp::SCOPE_PUBLIC;
		} else if (ctx->KEYWORD_PRIVATE() != nullptr) {
			current_method_visibility = bpp::SCOPE_PRIVATE;
		} else if (ctx->KEYWORD_PROTECTED() != nullptr) {
			current_method_visibility = bpp::SCOPE_PROTECTED;
		}

		// Is this method virtual?
		if (ctx->KEYWORD_VIRTUAL() != nullptr) {
			current_method_is_virtual = true;
		}

		current_method.set_name(current_method_name);
		current_method.set_scope(current_method_visibility);
		current_method.set_virtual(current_method_is_virtual);

	}
	void exitMethod_definition(BashppParser::Method_definitionContext *ctx) override {
		skip_comment

		current_method.set_method_body(current_method_body);
		current_class.add_method(current_method);

		std::cout << "Exited Method Definition" << std::endl;

		std::cout << "Class '" << current_class_name << "' has a " << current_method_visibility << " method named '" << current_method_name << "'" << std::endl;
		std::cout << "This method " << (current_method_is_virtual ? "is" : "is not") << " virtual" << std::endl;
		std::cout << "This method has " << current_method_parameters.size() << " parameters" << std::endl;
		for (bpp::bpp_method_parameter parameter : current_method_parameters) {
			std::cout << "Parameter: " << parameter.get_name() << " of type " << parameter.get_type() << std::endl;
		}

		current_method_name.clear();
		current_method_body.clear();
		current_method_visibility = bpp::SCOPE_PRIVATE;
		current_method_is_virtual = false;
		current_method_parameters.clear();
		in_method_definition = false;
	}

	void enterConstructor_definition(BashppParser::Constructor_definitionContext *ctx) override { 
		skip_comment
	}
	void exitConstructor_definition(BashppParser::Constructor_definitionContext *ctx) override { 
		skip_comment
	}

	void enterDestructor_definition(BashppParser::Destructor_definitionContext *ctx) override { 
		skip_comment
	}
	void exitDestructor_definition(BashppParser::Destructor_definitionContext *ctx) override { 
		skip_comment
	}

	void enterValue_assignment(BashppParser::Value_assignmentContext *ctx) override { 
		skip_comment
	}
	void exitValue_assignment(BashppParser::Value_assignmentContext *ctx) override { 
		skip_comment
	}

	void enterObject_instantiation(BashppParser::Object_instantiationContext *ctx) override {
		skip_comment

		std::cout << "Entered Object Instantiation" << std::endl;

		if (in_member_declaration) {
			current_member_type = ctx->IDENTIFIER()[0]->getText();
			current_member_name = ctx->IDENTIFIER()[1]->getText();
			current_member_is_pointer = false;

			// Get default value
			if (ctx->value_assignment() != nullptr) {
				current_member_default_value = ctx->value_assignment()->acceptable_rvalue()->getText();		
			}
			return;
		}
	}
	void exitObject_instantiation(BashppParser::Object_instantiationContext *ctx) override { 
		skip_comment
	}

	void enterPointer_declaration(BashppParser::Pointer_declarationContext *ctx) override {
		skip_comment

		std::cout << "Entered Pointer Declaration" << std::endl;

		if (in_member_declaration) {
			current_member_type = ctx->IDENTIFIER()[0]->getText();
			current_member_name = ctx->IDENTIFIER()[1]->getText();
			current_member_is_pointer = true;

			// Get default value
			if (ctx->value_assignment() != nullptr) {
				current_member_default_value = ctx->value_assignment()->acceptable_rvalue()->getText();
			}
		}
	}

	void exitPointer_declaration(BashppParser::Pointer_declarationContext *ctx) override { 
		skip_comment
	}

	void enterObject_assignment(BashppParser::Object_assignmentContext *ctx) override { 
		skip_comment
	}

	void exitObject_assignment(BashppParser::Object_assignmentContext *ctx) override { 
		skip_comment
	}

	void enterPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) override { 
		skip_comment
	}

	void exitPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) override { 
		skip_comment
	}

	void enterObject_address(BashppParser::Object_addressContext *ctx) override { 
		skip_comment
	}

	void exitObject_address(BashppParser::Object_addressContext *ctx) override { 
		skip_comment
	}

	void enterObject_reference(BashppParser::Object_referenceContext *ctx) override { 
		skip_comment
	}

	void exitObject_reference(BashppParser::Object_referenceContext *ctx) override { 
		skip_comment
	}

	void enterObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) override { 
		skip_comment
	}

	void exitObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) override { 
		skip_comment
	}

	void enterSelf_reference(BashppParser::Self_referenceContext *ctx) override { 
		skip_comment
	}

	void exitSelf_reference(BashppParser::Self_referenceContext *ctx) override { 
		skip_comment
	}

	void enterSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) override { 
		skip_comment
	}

	void exitSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) override {
		skip_comment
	}

	void enterNullptr_ref(BashppParser::Nullptr_refContext *ctx) override {
		skip_comment
	}

	void exitNullptr_ref(BashppParser::Nullptr_refContext *ctx) override {
		skip_comment
	}

	void enterNew_statement(BashppParser::New_statementContext *ctx) override {
		skip_comment
	}

	void exitNew_statement(BashppParser::New_statementContext *ctx) override {
		skip_comment
	}

	void enterDelete_statement(BashppParser::Delete_statementContext *ctx) override {
		skip_comment
	}

	void exitDelete_statement(BashppParser::Delete_statementContext *ctx) override {
		skip_comment
	}

	void enterSupershell(BashppParser::SupershellContext *ctx) override {
		skip_comment
	}

	void exitSupershell(BashppParser::SupershellContext *ctx) override {
		skip_comment
	}

	void enterSubshell(BashppParser::SubshellContext *ctx) override {
		skip_comment
	}

	void exitSubshell(BashppParser::SubshellContext *ctx) override {
		skip_comment
	}

	void enterDeprecated_subshell(BashppParser::Deprecated_subshellContext *ctx) override {
		skip_comment
	}

	void exitDeprecated_subshell(BashppParser::Deprecated_subshellContext *ctx) override {
		skip_comment
	}

	void enterString(BashppParser::StringContext *ctx) override {
		skip_comment
	}

	void exitString(BashppParser::StringContext *ctx) override {
		skip_comment
	}

	void enterSinglequote_string(BashppParser::Singlequote_stringContext *ctx) override {
		skip_comment
	}

	void exitSinglequote_string(BashppParser::Singlequote_stringContext *ctx) override {
		skip_comment
	}

	void enterComment(BashppParser::CommentContext *ctx) override { 
		in_comment = true;
	}

	void exitComment(BashppParser::CommentContext *ctx) override {
		in_comment = false;
	}

	void enterParameter(BashppParser::ParameterContext *ctx) override {
		skip_comment

		if (!in_method_definition) {
			std::cerr << "Error: Parameter outside of method definition" << std::endl;
			exit(1);
		}

		std::cout << "Entered Parameter" << std::endl;

		// Get parameter name & type
		// If it's a primitive, it will be a single IDENTIFIER
		// If it's an object, it will be two IDENTIFIERs -- the first one is the type, the second one is the name

		std::string parameter_type = "primitive";
		std::string parameter_name;

		if (ctx->IDENTIFIER().size() == 1) {
			parameter_name = ctx->IDENTIFIER()[0]->getText();
		} else {
			parameter_type = ctx->IDENTIFIER()[0]->getText();
			parameter_name = ctx->IDENTIFIER()[1]->getText();
		}

		bpp::bpp_method_parameter parameter(parameter_name);
		parameter.set_type(parameter_type);

		current_method.add_parameter(parameter);
	}

	void exitParameter(BashppParser::ParameterContext *ctx) override {
		skip_comment
	}

	void enterAcceptable_rvalue(BashppParser::Acceptable_rvalueContext *ctx) override {
		skip_comment
	}

	void exitAcceptable_rvalue(BashppParser::Acceptable_rvalueContext *ctx) override {
		skip_comment
	}

	void enterOther_statement(BashppParser::Other_statementContext *ctx) override {
		skip_comment

		std::cout << "Entered Other Statement" << std::endl;

		std::string statement = ctx->getText();
		std::cout << "Statement: " << statement << std::endl;

		if (in_method_definition) {
			current_method_body += statement;
			return;
		}

		program.add_code(statement);
	}

	void exitOther_statement(BashppParser::Other_statementContext *ctx) override {
		skip_comment
	}

};

#endif // ANTLR_BASHPPLISTENER_CPP_