/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterBashRedirection(std::shared_ptr<AST::BashRedirection> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Redirection outside of code entity");
	}

	// Just add it to the current code entity
	current_code_entity->add_code(node->OPERATOR().getValue() + " ", false);
}

void BashppListener::exitBashRedirection(std::shared_ptr<AST::BashRedirection> node) {}
