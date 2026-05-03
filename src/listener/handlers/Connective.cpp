/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <listener/BashppListener.h>

void BashppListener::enterConnective(std::shared_ptr<AST::Connective> node) {
	auto current_command_sequence = std::dynamic_pointer_cast<bpp::bash_command_sequence>(entity_stack.top());
	bpp_assert(current_command_sequence != nullptr, "Connective found outside of bash command sequence");

	bool is_and = (node->TYPE() == AST::Connective::ConnectiveType::AND);
	current_command_sequence->add_connective(is_and);
}

void BashppListener::exitConnective(std::shared_ptr<AST::Connective> node) {}
