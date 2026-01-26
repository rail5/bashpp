/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterHereString(std::shared_ptr<AST::HereString> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		syntax_error(node, "HereString outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> herestring_entity = std::make_shared<bpp::bpp_string>();
	herestring_entity->set_containing_class(current_code_entity->get_containing_class());
	herestring_entity->inherit(current_code_entity);

	entity_stack.push(herestring_entity);
}

void BashppListener::exitHereString(std::shared_ptr<AST::HereString> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> herestring_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (herestring_entity == nullptr) {
		throw internal_error("Herestring entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(herestring_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(herestring_entity->get_post_code());
	current_code_entity->add_code("<<<" + herestring_entity->get_code(), false);
}
