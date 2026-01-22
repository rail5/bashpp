/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBashRedirection(std::shared_ptr<AST::BashRedirection> node) {
	skip_syntax_errors

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Redirection outside of code entity");
	}

	// Just add it to the current code entity
	current_code_entity->add_code(node->OPERATOR().getValue(), false);
}

void BashppListener::exitBashRedirection(std::shared_ptr<AST::BashRedirection> node) {
	skip_syntax_errors
}
