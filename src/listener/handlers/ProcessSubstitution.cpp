/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) {
	skip_syntax_errors

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Redirection outside of code entity");
	}

	// Just add the substitution start token to the current code entity
	current_code_entity->add_code(node->SUBSTITUTIONSTART(), false);
}

void BashppListener::exitProcessSubstitution(std::shared_ptr<AST::ProcessSubstitution> node) {
	skip_syntax_errors

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Redirection outside of code entity");
	}

	// Add the substitution end token to the current code entity
	current_code_entity->add_code(")", false);
}
