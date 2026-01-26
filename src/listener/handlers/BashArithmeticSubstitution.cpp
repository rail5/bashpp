/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterBashArithmeticSubstitution(std::shared_ptr<AST::BashArithmeticSubstitution> node) {
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
		syntax_error(node, "Bash arithmetic outside of code entity");
	}

	// Create a new code entity for the arithmetic expression
	std::shared_ptr<bpp::bpp_string> arithmetic_entity = std::make_shared<bpp::bpp_string>();
	arithmetic_entity->set_containing_class(code_entity->get_containing_class());
	arithmetic_entity->inherit(code_entity);
	arithmetic_entity->set_requires_perfect_forwarding(true);

	// Push the arithmetic entity onto the entity stack
	entity_stack.push(arithmetic_entity);
}

void BashppListener::exitBashArithmeticSubstitution(std::shared_ptr<AST::BashArithmeticSubstitution> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> arithmetic_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (arithmetic_entity == nullptr) {
		throw internal_error("Bash arithmetic context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	current_code_entity->add_code_to_previous_line(arithmetic_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(arithmetic_entity->get_post_code());
	current_code_entity->add_code("$((" + arithmetic_entity->get_code() + "))");
}
