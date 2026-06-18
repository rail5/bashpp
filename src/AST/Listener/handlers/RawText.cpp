/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>

namespace bpp::AST {

template <>
void Listener::enter(RawText* node) {
	std::shared_ptr<bpp::IR::CodeEntity> current_code_entity = std::dynamic_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	if (!current_code_entity) {
		if (node->TEXT().getValue().find_first_not_of(" \t\n\r") == std::string::npos) return; // Harmless whitespace
		throw bpp::ErrorHandling::SyntaxError(this, node, "Executable code outside of code entity");
	}
	current_code_entity->add(node->TEXT().getValue());
}

template <>
void Listener::exit(RawText* node) {}

} // namespace bpp::AST
