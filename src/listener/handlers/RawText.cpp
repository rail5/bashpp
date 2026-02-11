/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterRawText(std::shared_ptr<AST::RawText> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		// Could just be harmless whitespace
		if (node->TEXT().getValue().find_first_not_of(" \t\n\r") == std::string::npos) return;
		throw bpp::ErrorHandling::SyntaxError(this, node, "Raw text outside of code entity");
	}

	// Just add it to the current code entity
	current_code_entity->add_code(node->TEXT().getValue());
}

void BashppListener::exitRawText(std::shared_ptr<AST::RawText> node) {}
