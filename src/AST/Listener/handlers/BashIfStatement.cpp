/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/BashIfStatement.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashIfStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "BashIfStatement node must be inside a code entity");

	auto if_statement_entity = std::make_unique<bpp::IR::BashIfStatement>();
	if_statement_entity->inherit(entity_stack.top());
	entity_stack.push(std::move(if_statement_entity));
}

template <>
void Listener::exit(BashIfStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfStatement>(), "Topmost entity is not a BashIfStatement when exiting BashIfStatement node");
	auto if_statement_entity = entity_stack.pop_as<bpp::IR::BashIfStatement>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashIfStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(if_statement_entity));
}

template <>
void Listener::enter(BashIfCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfBranch>(), "BashIfCondition node must be inside a BashIfBranch");
	auto* if_branch_entity = entity_stack.top_as<bpp::IR::BashIfBranch>();

	auto condition_entity = std::make_unique<bpp::IR::BashIfCondition>();
	condition_entity->inherit(if_branch_entity);

	entity_stack.push(std::move(condition_entity));
}

template <>
void Listener::exit(BashIfCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfCondition>(), "Topmost entity is not a BashIfCondition when exiting BashIfCondition node");
	auto condition_entity = entity_stack.pop_as<bpp::IR::BashIfCondition>();

	bpp_assert(topmost_entity_is<bpp::IR::BashIfBranch>(), "Topmost entity is not a BashIfBranch when exiting BashIfCondition node");
	auto* if_branch_entity = entity_stack.top_as<bpp::IR::BashIfBranch>();
	if_branch_entity->setCondition(std::move(condition_entity));
}

template <>
void Listener::enter(BashIfBranch* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfStatement>(), "Topmost entity is not a BashIfStatement when entering BashIfBranch node");
	auto* if_statement_entity = entity_stack.top_as<bpp::IR::BashIfStatement>();

	auto branch_entity = std::make_unique<bpp::IR::BashIfBranch>();
	branch_entity->inherit(if_statement_entity);
	branch_entity->setIsRoot(node->isRootBranch());

	entity_stack.push(std::move(branch_entity));
}

template <>
void Listener::exit(BashIfBranch* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfBranch>(), "Topmost entity is not a BashIfBranch when exiting BashIfBranch node");
	auto root_branch_entity = entity_stack.pop_as<bpp::IR::BashIfBranch>();

	bpp_assert(topmost_entity_is<bpp::IR::BashIfStatement>(), "Topmost entity is not a BashIfStatement when exiting BashIfBranch node");
	auto* if_statement_entity = entity_stack.top_as<bpp::IR::BashIfStatement>();
	if_statement_entity->addBranch(std::move(root_branch_entity));
}

} // namespace bpp::AST
