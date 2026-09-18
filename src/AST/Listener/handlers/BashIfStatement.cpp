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

	auto if_statement_entity = std::make_shared<bpp::IR::BashIfStatement>();
	if_statement_entity->inherit(entity_stack.top());
	entity_stack.push(if_statement_entity);
}

template <>
void Listener::exit(BashIfStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfStatement>(), "Topmost entity is not a BashIfStatement when exiting BashIfStatement node");
	auto if_statement_entity = std::static_pointer_cast<bpp::IR::BashIfStatement>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashIfStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(if_statement_entity);
}

template <>
void Listener::enter(BashIfCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfBranch>(), "BashIfCondition node must be inside a BashIfBranch");
	auto if_branch_entity = std::static_pointer_cast<bpp::IR::BashIfBranch>(entity_stack.top());

	auto condition_entity = std::make_shared<bpp::IR::BashIfCondition>();
	condition_entity->inherit(if_branch_entity);

	entity_stack.push(condition_entity);
}

template <>
void Listener::exit(BashIfCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfCondition>(), "Topmost entity is not a BashIfCondition when exiting BashIfCondition node");
	auto condition_entity = std::static_pointer_cast<bpp::IR::BashIfCondition>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::BashIfBranch>(), "Topmost entity is not a BashIfBranch when exiting BashIfCondition node");
	auto if_branch_entity = std::static_pointer_cast<bpp::IR::BashIfBranch>(entity_stack.top());
	if_branch_entity->setCondition(condition_entity);
}

template <>
void Listener::enter(BashIfBranch* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfStatement>(), "Topmost entity is not a BashIfStatement when entering BashIfBranch node");
	auto if_statement_entity = std::static_pointer_cast<bpp::IR::BashIfStatement>(entity_stack.top());

	auto branch_entity = std::make_shared<bpp::IR::BashIfBranch>();
	branch_entity->inherit(if_statement_entity);
	branch_entity->setIsRoot(node->isRootBranch());

	entity_stack.push(branch_entity);
}

template <>
void Listener::exit(BashIfBranch* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashIfBranch>(), "Topmost entity is not a BashIfBranch when exiting BashIfBranch node");
	auto root_branch_entity = std::static_pointer_cast<bpp::IR::BashIfBranch>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::BashIfStatement>(), "Topmost entity is not a BashIfStatement when exiting BashIfBranch node");
	auto if_statement_entity = std::static_pointer_cast<bpp::IR::BashIfStatement>(entity_stack.top());
	if_statement_entity->addBranch(root_branch_entity);
}

} // namespace bpp::AST
