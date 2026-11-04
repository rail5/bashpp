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
void Listener::enter(ParameterExpansion* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	// Just add the start token to the current code entity
	// FIXME(@rail5): Handle parameter expansion properly (esp. for nonprimitive references) rather than just throwing up our hands like this
	current_code_entity->add(node->EXPANSIONBEGIN().getValue());
}

template <>
void Listener::exit(ParameterExpansion* /*node*/) {}

} // namespace bpp::AST
