/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_
#define SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterConstructor_definition(BashppParser::Constructor_definitionContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Constructor definitions take the form
	 * 	@constructor { ... }
	 */

	// Verify that we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw_syntax_error(ctx->KEYWORD_CONSTRUCTOR(), "Constructor definition outside of class");
	}

	// Verify that the constructor hasn't already been set
	if (current_class->has_constructor()) {
		throw_syntax_error(ctx->KEYWORD_CONSTRUCTOR(), "Constructor already defined");
	}

	std::shared_ptr<bpp::bpp_constructor> constructor = std::make_shared<bpp::bpp_constructor>();
	constructor->set_containing_class(current_class);
	constructor->inherit(program);
	entity_stack.push(constructor);
}

void BashppListener::exitConstructor_definition(BashppParser::Constructor_definitionContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_constructor> constructor = std::dynamic_pointer_cast<bpp::bpp_constructor>(entity_stack.top());
	if (constructor == nullptr) {
		throw internal_error("Constructor definition not found on the entity stack", ctx);
	}

	entity_stack.pop();

	// Call destructors for any objects created in the constructor before we exit it
	constructor->destruct_local_objects();

	// Add the constructor to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw internal_error("Class not found on the entity stack", ctx);
	}

	current_class->set_constructor(constructor);
}

#endif // SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_
