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
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto process_substitution_entity = std::make_unique<bpp::IR::StringType>();
	process_substitution_entity->inherit(current_code_entity);

	process_substitution_entity->add(node->SUBSTITUTIONSTART());

	entity_stack.push(std::move(process_substitution_entity));
}

template <>
void Listener::exit(ProcessSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting ProcessSubstitution node");
	auto process_substitution_entity = entity_stack.pop_as<bpp::IR::StringType>();

	process_substitution_entity->add(")"); // All process substitutions end with a closing parenthesis

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting ProcessSubstitution node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(process_substitution_entity));
}

} // namespace bpp::AST
