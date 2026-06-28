/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashRedirection* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashRedirection node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	// Just add the redirection operator as RawCode to the current code entity
	current_code_entity->add(node->OPERATOR().getValue() + " ");
}

template <>
void Listener::exit(BashRedirection* /*node*/) {}

} // namespace bpp::AST
