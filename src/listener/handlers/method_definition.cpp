/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_METHOD_DEFINITION_CPP_
#define SRC_LISTENER_HANDLERS_METHOD_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterMethod_definition(BashppParser::Method_definitionContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	// Verify we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (current_class == nullptr) {
		throw_syntax_error(ctx->KEYWORD_METHOD(), "Method definition outside of class body");
	}

	std::string method_name = ctx->IDENTIFIER()->getText();

	// Add the method to entity stack
	std::shared_ptr<bpp::bpp_method> method = std::make_shared<bpp::bpp_method>(method_name);
	method->inherit(program);
	method->set_containing_class(current_class);

	// Set the method's scope
	if (ctx->KEYWORD_PUBLIC() != nullptr) {
		method->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
	} else if (ctx->KEYWORD_PROTECTED() != nullptr) {
		method->set_scope(bpp::bpp_scope::SCOPE_PROTECTED);
	} else {
		method->set_scope(bpp::bpp_scope::SCOPE_PRIVATE);
	}

	// Virtual?
	if (ctx->KEYWORD_VIRTUAL() != nullptr) {
		method->set_virtual(true);
	}

	entity_stack.push(method);
}

void BashppListener::exitMethod_definition(BashppParser::Method_definitionContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	// Get the method from the entity stack
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	entity_stack.pop();

	// Call destructors for any objects created in the method before we exit it
	method->destruct_local_objects();

	// If the method is toPrimitive, verify that the scope is public
	if (method->get_name() == "toPrimitive" && method->get_scope() != bpp::bpp_scope::SCOPE_PUBLIC) {
		antlr4::tree::TerminalNode* scope_keyword;
		if (ctx->KEYWORD_PUBLIC() != nullptr) {
			scope_keyword = ctx->KEYWORD_PUBLIC();
		} else if (ctx->KEYWORD_PROTECTED() != nullptr) {
			scope_keyword = ctx->KEYWORD_PROTECTED();
		} else {
			scope_keyword = ctx->KEYWORD_PRIVATE();
		}
		throw_syntax_error_from_exitRule(scope_keyword, "toPrimitive method must be public");
		return;
	}

	// Add the method to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw internal_error("Current class was not found in the entity stack", ctx);
	}

	if (!current_class->add_method(method)) {
		throw_syntax_error_from_exitRule(ctx->IDENTIFIER(), "Method redefinition: " + method->get_name());
	}
}

#endif // SRC_LISTENER_HANDLERS_METHOD_DEFINITION_CPP_
