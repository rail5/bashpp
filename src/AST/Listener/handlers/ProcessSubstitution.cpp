/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(ProcessSubstitution* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering ProcessSubstitution node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto process_substitution_entity = std::make_shared<bpp::IR::StringType>();
	process_substitution_entity->inherit(current_code_entity);

	process_substitution_entity->add(node->SUBSTITUTIONSTART());

	entity_stack.push(process_substitution_entity);
}

template <>
void Listener::exit(ProcessSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting ProcessSubstitution node");
	auto process_substitution_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	process_substitution_entity->add(")"); // All process substitutions end with a closing parenthesis

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting ProcessSubstitution node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(process_substitution_entity);
}

} // namespace bpp::AST
