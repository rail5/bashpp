/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_METHOD_DEFINITION_CPP_
#define ANTLR_LISTENER_HANDLERS_METHOD_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterMethod_definition(BashppParser::Method_definitionContext *ctx) {
	skip_comment
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
	entity_stack.push(method);
}

void BashppListener::exitMethod_definition(BashppParser::Method_definitionContext *ctx) {
	skip_comment
	skip_singlequote_string

	// Get the method from the entity stack
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	entity_stack.pop();

	// Call destructors for any objects created in the method before we exit it
	method->destruct_local_objects();

	// Add the method to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (!current_class->add_method(method)) {
		throw_syntax_error(ctx->IDENTIFIER(), "Method redefinition: " + method->get_name());
	}
}

#endif // ANTLR_LISTENER_HANDLERS_METHOD_DEFINITION_CPP_
