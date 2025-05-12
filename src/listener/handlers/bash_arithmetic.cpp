/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_BASH_ARITHMETIC_CPP_
#define SRC_LISTENER_HANDLERS_BASH_ARITHMETIC_CPP_

#include "../BashppListener.h"

void BashppListener::enterBash_arithmetic(BashppParser::Bash_arithmeticContext *ctx) {
	skip_syntax_errors
	/**
	 * Bash arithmetic is a series of arithmetic operations
	 * that are enclosed in $((...))
	 * They do not run in a subshell. So, unlike with the subshell rule,
	 * We can preserve objects instantiated within the arithmetic context, etc
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw_syntax_error(ctx->BASH_ARITH_START(), "Bash arithmetic outside of code entity");
	}

	// Create a new code entity for the arithmetic expression
	std::shared_ptr<bpp::bpp_string> arithmetic_entity = std::make_shared<bpp::bpp_string>();
	arithmetic_entity->set_containing_class(code_entity->get_containing_class());
	arithmetic_entity->inherit(code_entity);

	// Push the arithmetic entity onto the entity stack
	entity_stack.push(arithmetic_entity);
}

void BashppListener::exitBash_arithmetic(BashppParser::Bash_arithmeticContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> arithmetic_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (arithmetic_entity == nullptr) {
		throw internal_error("Bash arithmetic context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	current_code_entity->add_code_to_previous_line(arithmetic_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(arithmetic_entity->get_post_code());
	current_code_entity->add_code(ctx->BASH_ARITH_START()->getText() + arithmetic_entity->get_code() + "))");
}

#endif // SRC_LISTENER_HANDLERS_BASH_ARITHMETIC_CPP_
