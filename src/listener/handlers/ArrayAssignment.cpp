/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterArrayAssignment(std::shared_ptr<AST::ArrayAssignment> node) {
	skip_syntax_errors
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		syntax_error(node, "Array assignment outside of code entity");
	}

	std::shared_ptr<bpp::bpp_string> array_assignment_entity = std::make_shared<bpp::bpp_string>();
	array_assignment_entity->set_containing_class(code_entity->get_containing_class());
	array_assignment_entity->inherit(code_entity);
	array_assignment_entity->set_requires_perfect_forwarding(true);

	entity_stack.push(array_assignment_entity);

	array_assignment_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitArrayAssignment(std::shared_ptr<AST::ArrayAssignment> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> array_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (array_assignment_entity == nullptr) {
		throw internal_error("Array assignment context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	auto value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());
	if (value_assignment_entity != nullptr) {
		value_assignment_entity->set_array_assignment(true);
	}

	current_code_entity->add_code_to_previous_line(array_assignment_entity->get_pre_code());
	current_code_entity->add_code_to_next_line("\n" + array_assignment_entity->get_post_code());
	current_code_entity->add_code("(" + array_assignment_entity->get_code() + ")");

	program->mark_entity(
		source_file,
		array_assignment_entity->get_initial_definition().line,
		array_assignment_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		array_assignment_entity
	);
}
