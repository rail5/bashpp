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
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto arithmetic_substitution_entity = std::make_shared<bpp::IR::StringType>();
	arithmetic_substitution_entity->inherit(current_code_entity);
	arithmetic_substitution_entity->add("$(("); // Copy the `$((` start token as RawCode into the entity
	entity_stack.push(arithmetic_substitution_entity);
}

template <>
void Listener::exit(BashArithmeticSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashArithmeticSubstitution node");
	auto arithmetic_substitution_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	arithmetic_substitution_entity->add("))"); // Copy the `))` end token as RawCode into the entity

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashArithmeticSubstitution node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(arithmetic_substitution_entity);
}

} // namespace bpp::AST
