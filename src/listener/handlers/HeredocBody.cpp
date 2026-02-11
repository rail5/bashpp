/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterHeredocBody(std::shared_ptr<AST::HeredocBody> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Heredoc outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::make_shared<bpp::bpp_string>();
	heredoc_entity->set_containing_class(current_code_entity->get_containing_class());
	heredoc_entity->inherit(current_code_entity);

	entity_stack.push(heredoc_entity);
}

void BashppListener::exitHeredocBody(std::shared_ptr<AST::HeredocBody> node) {
	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (heredoc_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Heredoc entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(heredoc_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(heredoc_entity->get_post_code());
	current_code_entity->add_code("\n" + heredoc_entity->get_code() + node->DELIMITER().getValue(), false);
}
