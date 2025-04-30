/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_
#define SRC_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
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

	std::shared_ptr<bpp::bpp_method> destructor = std::make_shared<bpp::bpp_method>();
	destructor->set_name("__destructor");
	destructor->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
	destructor->set_virtual(true);
	destructor->set_containing_class(current_class);
	destructor->inherit(program);
	entity_stack.push(destructor);
}

void BashppListener::exitDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_method> destructor = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
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

	if (!current_class->add_method(destructor)) {
		throw_syntax_error_from_exitRule(ctx->KEYWORD_DESTRUCTOR(), "Destructor already defined");
	}
}

#endif // SRC_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_
