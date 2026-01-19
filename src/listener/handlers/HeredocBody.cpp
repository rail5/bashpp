/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterHeredocBody(std::shared_ptr<AST::HeredocBody> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Heredoc outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::make_shared<bpp::bpp_string>();
	heredoc_entity->set_containing_class(current_code_entity->get_containing_class());
	heredoc_entity->inherit(current_code_entity);

	entity_stack.push(heredoc_entity);
}

void BashppListener::exitHeredocBody(std::shared_ptr<AST::HeredocBody> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (heredoc_entity == nullptr) {
		throw internal_error("Heredoc entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(heredoc_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(heredoc_entity->get_post_code());
	// TODO(@rail5): Check: Do we need to prepend a newline?
	current_code_entity->add_code(heredoc_entity->get_code() + node->DELIMITER().getValue(), false);
}
