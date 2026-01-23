/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node) {
	skip_syntax_errors

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack");
	}

	// Just add the start token to the current code entity
	// TODO(@rail5): Make this more robust, handle parameter expansion properly instead of just throwing up our hands like this
	current_code_entity->add_code(node->EXPANSIONBEGIN(), false);
}

void BashppListener::exitParameterExpansion(std::shared_ptr<AST::ParameterExpansion> node) {
	skip_syntax_errors
}
