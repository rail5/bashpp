/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterBashVariable(std::shared_ptr<AST::BashVariable> node) {
	skip_syntax_errors

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		syntax_error(node, "Command outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> bash_variable_entity = std::make_shared<bpp::bpp_string>();
	bash_variable_entity->set_containing_class(current_code_entity->get_containing_class());
	bash_variable_entity->inherit(current_code_entity);
	entity_stack.push(bash_variable_entity);
}

void BashppListener::exitBashVariable(std::shared_ptr<AST::BashVariable> node) {
	skip_syntax_errors

	auto bash_variable_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (bash_variable_entity == nullptr) {
		throw internal_error("Bash variable context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(bash_variable_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(bash_variable_entity->get_post_code());
	current_code_entity->add_code("${" + node->TEXT().getValue() + bash_variable_entity->get_code() + "}");
}
