/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) {
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Process substitution outside of code entity");
	}

	auto substitution_entity = std::make_shared<bpp::bpp_string>();
	substitution_entity->set_containing_class(current_code_entity->get_containing_class());
	substitution_entity->inherit(current_code_entity);
	substitution_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);

	entity_stack.push(substitution_entity);

	// Add the substitution start token to the current code entity
	current_code_entity->add_code(node->SUBSTITUTIONSTART(), false);
}

void BashppListener::exitProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) {
	auto substitution_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (substitution_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Process substitution context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Process substitution outside of code entity");
	}

	substitution_entity->destruct_local_objects(program);
	substitution_entity->flush_code_buffers();

	current_code_entity->add_code(substitution_entity->get_pre_code());
	current_code_entity->add_code(substitution_entity->get_code());
	current_code_entity->add_code(substitution_entity->get_post_code());

	// Add the substitution end token to the current code entity
	current_code_entity->add_code(")", false);
}
