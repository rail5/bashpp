/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <error/InternalError.h>

#include <IR/entities/expressions/bash/BashArithmeticForStatement.h>

namespace bpp::AST {

template <>
void Listener::enter(BashArithmeticForStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto arithmetic_for_statement_entity = std::make_unique<bpp::IR::BashArithmeticForStatement>();
	arithmetic_for_statement_entity->inherit(current_code_entity);
	entity_stack.push(std::move(arithmetic_for_statement_entity));
}

template <>
void Listener::exit(BashArithmeticForStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashArithmeticForStatement>(), "Topmost entity is not a BashArithmeticForStatement");
	auto arithmetic_for_statement_entity = entity_stack.pop_as<bpp::IR::BashArithmeticForStatement>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(arithmetic_for_statement_entity));
}

template <>
void Listener::enter(BashArithmeticForCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashArithmeticForStatement>(), "Topmost entity is not a BashArithmeticForStatement");
	auto* arithmetic_for_statement_entity = entity_stack.top_as<bpp::IR::BashArithmeticForStatement>();

	auto condition_entity = std::make_unique<bpp::IR::StringType>();
	condition_entity->inherit(arithmetic_for_statement_entity);
	entity_stack.push(std::move(condition_entity));
}

template <>
void Listener::exit(BashArithmeticForCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType");
	auto condition_entity = entity_stack.pop_as<bpp::IR::StringType>();
	bpp_assert(topmost_entity_is<bpp::IR::BashArithmeticForStatement>(), "Topmost entity is not a BashArithmeticForStatement");
	auto* arithmetic_for_statement_entity = entity_stack.top_as<bpp::IR::BashArithmeticForStatement>();
	arithmetic_for_statement_entity->setCondition(std::move(condition_entity));
}

// No need to handle BashArithmeticStatement specially,
// Since they're StringType nodes, they have inner nodes which will call ->add()
// on the containing ForCondition automatically.

} // namespace bpp::AST
