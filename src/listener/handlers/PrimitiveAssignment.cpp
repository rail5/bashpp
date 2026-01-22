/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) {
	skip_syntax_errors

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Variable assignment outside of code entity");
	}

	auto assignment_entity = std::make_shared<bpp::bpp_string>();
	assignment_entity->set_containing_class(current_code_entity->get_containing_class());
	assignment_entity->inherit(current_code_entity);
	entity_stack.push(assignment_entity);
}

void BashppListener::exitPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) {
	skip_syntax_errors

	auto assignment_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (assignment_entity == nullptr) {
		throw internal_error("Primitive assignment context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(assignment_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(assignment_entity->get_post_code());
	current_code_entity->add_code(node->IDENTIFIER().getValue() + "=" + assignment_entity->get_code());
}
