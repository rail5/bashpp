/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_
#define SRC_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Destructor definitions take the form
	 * 	@destructor { ... }
	 */

	// Verify that we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw_syntax_error(ctx->KEYWORD_DESTRUCTOR(), "Destructor definition outside of class");
	}

	// Verify that the destructor hasn't already been set
	if (current_class->has_destructor()) {
		throw_syntax_error(ctx->KEYWORD_DESTRUCTOR(), "Destructor already defined");
	}

	std::shared_ptr<bpp::bpp_destructor> destructor = std::make_shared<bpp::bpp_destructor>();
	destructor->set_containing_class(current_class);
	destructor->inherit(program);
	entity_stack.push(destructor);
}

void BashppListener::exitDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_destructor> destructor = std::dynamic_pointer_cast<bpp::bpp_destructor>(entity_stack.top());
	if (destructor == nullptr) {
		throw internal_error("Destructor definition not found on the entity stack", ctx);
	}

	entity_stack.pop();

	// Call destructors for any objects created in the destructor before we exit it
	destructor->destruct_local_objects();

	// Add the destructor to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw internal_error("Class not found on the entity stack", ctx);
	}

	current_class->set_destructor(destructor);
}

#endif // SRC_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_
