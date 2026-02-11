/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) {
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Variable assignment outside of code entity");
	}

	auto assignment_entity = std::make_shared<bpp::bpp_string>();
	assignment_entity->set_containing_class(current_code_entity->get_containing_class());
	assignment_entity->inherit(current_code_entity);
	entity_stack.push(assignment_entity);
}

void BashppListener::exitPrimitiveAssignment(std::shared_ptr<AST::PrimitiveAssignment> node) {
	auto assignment_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (assignment_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Primitive assignment context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}

	std::string assignment_lvalue;
	if (node->isLocal()) assignment_lvalue += "local ";
	assignment_lvalue += node->IDENTIFIER().getValue();

	current_code_entity->add_code_to_previous_line(assignment_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(assignment_entity->get_post_code());
	current_code_entity->add_code(assignment_lvalue + assignment_entity->get_code());
}
