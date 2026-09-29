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
void Listener::enter(BashArithmeticSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashArithmeticSubstitution node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto arithmetic_substitution_entity = std::make_unique<bpp::IR::StringType>();
	arithmetic_substitution_entity->inherit(current_code_entity);
	arithmetic_substitution_entity->add("$(("); // Copy the `$((` start token as RawCode into the entity
	entity_stack.push(std::move(arithmetic_substitution_entity));
}

template <>
void Listener::exit(BashArithmeticSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashArithmeticSubstitution node");
	auto arithmetic_substitution_entity = entity_stack.pop_as<bpp::IR::StringType>();

	arithmetic_substitution_entity->add("))"); // Copy the `))` end token as RawCode into the entity

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashArithmeticSubstitution node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(arithmetic_substitution_entity));
}

} // namespace bpp::AST
