/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BASHPPLISTENER_CPP_
#define ANTLR_BASHPPLISTENER_CPP_

#include <iostream>
#include <vector>
#include <set>
#include <memory>
#include <stack>
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

#include "syntax_error.cpp"
#include "internal_error.cpp"

#define skip_comment if (in_comment) return;

#define throw_syntax_error(token, msg) antlr4::Token* symbol = token->getSymbol(); \
			int line = symbol->getLine(); \
			int column = symbol->getCharPositionInLine(); \
			throw syntax_error(msg, source_file, line, column);

class BashppListener : public BashppParserBaseListener {
	private:
		std::string source_file;
		bpp::bpp_program program;

		bool in_comment = false;
		bool in_singlequote_string = false;

		std::stack<std::shared_ptr<bpp::bpp_entity>> entity_stack;
		// The entity_stack is used to keep track of the current entity being processed

		std::set<std::string> protected_keywords = {
			"class", "constructor", "delete", "destructor",
			"include", "method", "new", "nullptr",
			"primitive", "private", "protected", "public",
			"this", "virtual"
		};

		bool in_value_assignment = false;
		std::string value_assignment = "";
		std::string pre_valueassignment_code = "";
		std::string post_valueassignment_code = "";
		
	public:

	void set_source_file(std::string source_file) {
		this->source_file = source_file;
	}

	void enterProgram(BashppParser::ProgramContext *ctx) override {
		program.add_code("#!/usr/bin/env bash\n");
		program.add_code(bpp_supershell_function);

		entity_stack.push(std::make_shared<bpp::bpp_program>());
	}
	void exitProgram(BashppParser::ProgramContext *ctx) override {
		std::cout << program.get_code() << std::endl;
	}

	void enterInclude_statement(BashppParser::Include_statementContext *ctx) override { 
		skip_comment
	}
	void exitInclude_statement(BashppParser::Include_statementContext *ctx) override { 
		skip_comment
	}

	void enterClass_definition(BashppParser::Class_definitionContext *ctx) override {
		skip_comment

		std::shared_ptr<bpp::bpp_class> new_class = std::make_shared<bpp::bpp_class>();
		entity_stack.push(new_class);

		// Get the class name
		std::string class_name = ctx->IDENTIFIER(0)->getText();

		// Verify that the class name is not already in use (or a protected keyword)
		if (protected_keywords.find(class_name) != protected_keywords.end()) {
			throw_syntax_error(ctx->IDENTIFIER(0), "Invalid class name: " + class_name);
		}

		if (program.get_class(class_name) != nullptr) {
			throw_syntax_error(ctx->IDENTIFIER(0), "Class already exists: " + class_name);
		}

		if (program.get_object(class_name) != nullptr) {
			throw_syntax_error(ctx->IDENTIFIER(0), "Object already exists: " + class_name);
		}

		new_class->set_name(class_name);

		// Inherit from a parent class if specified
		if (ctx->IDENTIFIER().size() > 1) {
			std::string parent_class_name = ctx->IDENTIFIER(1)->getText();
			std::shared_ptr<bpp::bpp_class> parent_class = program.get_class(parent_class_name);
			if (parent_class == nullptr) {
				throw_syntax_error(ctx->IDENTIFIER(1), "Parent class not found: " + parent_class_name);
			}
			new_class->inherit(parent_class);
		}
	}
	void exitClass_definition(BashppParser::Class_definitionContext *ctx) override {
		skip_comment

		std::shared_ptr<bpp::bpp_class> new_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

		if (new_class == nullptr) {
			throw internal_error("entity_stack top is not a bpp_class");
		}

		entity_stack.pop();

		// Add the class to the program
		program.add_class(new_class);
	}

	void enterMember_declaration(BashppParser::Member_declarationContext *ctx) override {
		skip_comment

		std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

		if (current_class == nullptr) {
			throw_syntax_error(ctx->AT(), "Member declaration outside of class");
		}

		std::shared_ptr<bpp::bpp_datamember> new_datamember = std::make_shared<bpp::bpp_datamember>();
		entity_stack.push(new_datamember);

		// Get visibility
		// One of KEYWORD_PUBLIC, KEYWORD_PRIVATE, KEYWORD_PROTECTED will be set
		if (ctx->KEYWORD_PUBLIC() != nullptr) {
			new_datamember->set_scope(bpp::SCOPE_PUBLIC);
		} else if (ctx->KEYWORD_PRIVATE() != nullptr) {
			new_datamember->set_scope(bpp::SCOPE_PRIVATE);
		} else if (ctx->KEYWORD_PROTECTED() != nullptr) {
			new_datamember->set_scope(bpp::SCOPE_PROTECTED);
		}

		/**
		 * This will either be:
		 * 	1. A primitive
		 * 	2. An object
		 * 	3. A pointer
		 * If it's a primitive, then IDENTIFIER will be set
		 * If it's an object, then object_instantiation will be set, and we'll handle that in the object_instantiation rule
		 * If it's a pointer, then pointer_declaration will be set, and we'll handle that in the pointer_declaration rule
		 */

		if (ctx->IDENTIFIER() != nullptr) {
			// It's a primitive
			std::string member_name = ctx->IDENTIFIER()->getText();
			new_datamember->set_name(member_name);
			new_datamember->set_type("primitive");
		}
	}
	void exitMember_declaration(BashppParser::Member_declarationContext *ctx) override {
		skip_comment

		std::shared_ptr<bpp::bpp_datamember> new_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
		if (new_datamember == nullptr) {
			throw internal_error("entity_stack top is not a bpp_datamember");
		}

		entity_stack.pop();

		std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

		current_class->add_datamember(new_datamember);
	}

	void enterObject_instantiation(BashppParser::Object_instantiationContext *ctx) override {
		skip_comment

		/**
		 * The object type will be stored in one of either IDENTIFIER_LVALUE or IDENTIFIER(0)
		 * If IDENTIFIER_LVALUE, then the object name will be in IDENTIFIER(0)
		 * If IDENTIFIER(0), then the object name will be in IDENTIFIER(1)
		 */

		// Verify that we're in a place where an object *can* be instantiated
		std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
		if (current_class != nullptr) {
			throw_syntax_error(ctx->AT(), "Stray object instantiation inside class body.\nDid you mean to declare a data member?\nIf so, start by declaring the data member with a visibility keyword (@public, @private, @protected)");
		}

		std::shared_ptr<bpp::bpp_object> new_object = std::make_shared<bpp::bpp_object>();
		entity_stack.push(new_object);

		antlr4::tree::TerminalNode* object_type = nullptr;
		antlr4::tree::TerminalNode* object_name = nullptr;

		if (ctx->IDENTIFIER_LVALUE() != nullptr) {
			// The object type is in IDENTIFIER_LVALUE
			object_type = ctx->IDENTIFIER_LVALUE();
			object_name = ctx->IDENTIFIER(0);
		} else {
			// The object type is in IDENTIFIER(0)
			object_type = ctx->IDENTIFIER(0);
			object_name = ctx->IDENTIFIER(1);
		}

		new_object->set_name(object_name->getText());
		new_object->set_class(program.get_class(object_type->getText()));

		// Verify that the object's class exists
		if (new_object->get_class() == nullptr) {
			throw_syntax_error(object_type, "Class not found: " + object_type->getText());
		}

		// Verify that the object's name is not already in use (or a protected keyword)
		if (protected_keywords.find(new_object->get_name()) != protected_keywords.end()) {
			throw_syntax_error(object_name, "Invalid object name: " + new_object->get_name());
		}

		if (program.get_class(new_object->get_name()) != nullptr) {
			throw_syntax_error(object_name, "Class already exists: " + new_object->get_name());
		}

		if (program.get_object(new_object->get_name()) != nullptr) {
			throw_syntax_error(object_name, "Object already exists: " + new_object->get_name());
		}
	}
	void exitObject_instantiation(BashppParser::Object_instantiationContext *ctx) override { 
		skip_comment

		std::shared_ptr<bpp::bpp_object> new_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
		if (new_object == nullptr) {
			throw internal_error("entity_stack top is not a bpp_object");
		}

		entity_stack.pop();

		std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
		if (current_datamember != nullptr) {
			// We're midway through a class member declaration
			// The data for this object should be moved to the datamember
			current_datamember->set_type(new_object->get_class()->get_name());
			current_datamember->set_name(new_object->get_name());
		}

		std::shared_ptr<bpp::bpp_method> current_method = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
		if (current_method != nullptr) {
			// We're midway through a method definition
			// The data for this object should be moved to the method body
		}

		// Add the object to the program
	}

	void enterPointer_declaration(BashppParser::Pointer_declarationContext *ctx) override {
		skip_comment
	}
	void exitPointer_declaration(BashppParser::Pointer_declarationContext *ctx) override { 
		skip_comment
	}

	void enterValue_assignment(BashppParser::Value_assignmentContext *ctx) override { 
		skip_comment
		in_value_assignment = true;
		pre_valueassignment_code.clear();
		post_valueassignment_code.clear();
		value_assignment.clear();
	}
	void exitValue_assignment(BashppParser::Value_assignmentContext *ctx) override { 
		skip_comment
		in_value_assignment = false;

		/**
		 * Value assignments will appear in the following contexts:
		 * 	1. Member declarations
		 * 	2. Object instantiations
		 * 	3. Pointer declarations
		 * 	4. Object assignments
		 */

		// Check if we're in a member declaration
		std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
		if (current_datamember != nullptr) {
			current_datamember->set_default_value(value_assignment);
			current_datamember->set_pre_access_code(pre_valueassignment_code);
			current_datamember->set_post_access_code(post_valueassignment_code);
			return;
		}

		// Check if we're in an object instantiation or pointer declaration
		std::shared_ptr<bpp::bpp_object> current_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
		if (current_object != nullptr) {
			// Is it a pointer?
			if (current_object->is_pointer()) {
				current_object->set_address(value_assignment);
			}
			// Else
		}

		// Check if we're in an object assignment
	
	}

	void enterMethod_definition(BashppParser::Method_definitionContext *ctx) override {
		skip_comment
	}
	void exitMethod_definition(BashppParser::Method_definitionContext *ctx) override {
		skip_comment
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
		in_singlequote_string = true;
	}
	void exitSinglequote_string(BashppParser::Singlequote_stringContext *ctx) override {
		skip_comment
		in_singlequote_string = false;
	}

	void enterComment(BashppParser::CommentContext *ctx) override { 
		in_comment = true;
	}
	void exitComment(BashppParser::CommentContext *ctx) override {
		in_comment = false;
	}

	void enterParameter(BashppParser::ParameterContext *ctx) override {
		skip_comment
	}
	void exitParameter(BashppParser::ParameterContext *ctx) override {
		skip_comment
	}

	void enterOther_statement(BashppParser::Other_statementContext *ctx) override {
		skip_comment
	}
	void exitOther_statement(BashppParser::Other_statementContext *ctx) override {
		skip_comment
	}

	void enterRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) override {
		skip_comment
		if (!in_value_assignment) {
			return;
		}

		// One of either IDENTIFIER, NUMBER, or BASH_VAR will be set
		if (ctx->IDENTIFIER() != nullptr) {
			value_assignment += ctx->IDENTIFIER()->getText();
		} else if (ctx->NUMBER() != nullptr) {
			value_assignment += ctx->NUMBER()->getText();
		} else if (ctx->BASH_VAR() != nullptr) {
			value_assignment += ctx->BASH_VAR()->getText();
		}
	}
	void exitRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) override {
		skip_comment
	}

};

#endif // ANTLR_BASHPPLISTENER_CPP_