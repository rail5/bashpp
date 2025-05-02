/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_
#define SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterConstructor_definition(BashppParser::Constructor_definitionContext *ctx) {
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

	std::shared_ptr<bpp::bpp_method> constructor = std::make_shared<bpp::bpp_method>();
	constructor->set_name("__constructor");
	constructor->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
	constructor->set_virtual(true);
	constructor->set_containing_class(current_class);
	constructor->inherit(program);
	entity_stack.push(constructor);
}

void BashppListener::exitConstructor_definition(BashppParser::Constructor_definitionContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_method> constructor = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	if (constructor == nullptr) {
		throw internal_error("Constructor definition not found on the entity stack", ctx);
	}

	entity_stack.pop();

	// Call destructors for any objects created in the constructor before we exit it
	constructor->destruct_local_objects(program);

	// Add the constructor to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw internal_error("Class not found on the entity stack", ctx);
	}

	if (!current_class->add_method(constructor)) {
		throw_syntax_error_from_exitRule(ctx->KEYWORD_CONSTRUCTOR(), "Constructor already defined");
	}
}

#endif // SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_
