/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterTypeofExpression(std::shared_ptr<AST::TypeofExpression> node) {
	skip_syntax_errors
	/**
	 * The typeof statement is used to determine the type of a value at runtime.
	 * It takes the form:
	 * 	@typeof <value>
	 * Where <value> is a valid rvalue.
	 */
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Typeof expression outside of code entity");
	}

	std::shared_ptr<bpp::bpp_string> typeof_entity = std::make_shared<bpp::bpp_string>();
	typeof_entity->set_containing_class(current_code_entity->get_containing_class());
	typeof_entity->inherit(current_code_entity);

	entity_stack.push(typeof_entity);
	typeof_stack.push({});
}

void BashppListener::exitTypeofExpression(std::shared_ptr<AST::TypeofExpression> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> typeof_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (typeof_entity == nullptr) {
		throw internal_error("Typeof context was not found in the entity stack");
	}

	entity_stack.pop();
	typeof_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack");
	}

	code_segment typeof_code = bpp::generate_typeof_code(typeof_entity->get_code(), program);

	current_code_entity->add_code_to_previous_line(typeof_entity->get_pre_code());
	current_code_entity->add_code_to_previous_line(typeof_code.pre_code);

	current_code_entity->add_code_to_next_line(typeof_entity->get_post_code());
	current_code_entity->add_code_to_next_line(typeof_code.post_code);

	current_code_entity->add_code(typeof_code.code);
}
