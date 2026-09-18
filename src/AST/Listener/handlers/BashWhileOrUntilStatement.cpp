/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/BashWhileOrUntilStatement.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashWhileOrUntilCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when entering BashWhileOrUntilCondition node");
	auto while_or_until_entity = std::static_pointer_cast<bpp::IR::BashWhileOrUntilStatement>(entity_stack.top());

	auto condition_entity = std::make_shared<bpp::IR::StringType>();
	condition_entity->inherit(while_or_until_entity);

	entity_stack.push(condition_entity);
}

template <>
void Listener::exit(BashWhileOrUntilCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashWhileOrUntilCondition node");
	auto condition_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when exiting BashWhileOrUntilCondition node");
	auto while_or_until_entity = std::static_pointer_cast<bpp::IR::BashWhileOrUntilStatement>(entity_stack.top());

	while_or_until_entity->setCondition(condition_entity);
}

template <>
void Listener::enter(BashWhileStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashWhileStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto while_entity = std::make_shared<bpp::IR::BashWhileOrUntilStatement>();
	while_entity->inherit(current_code_entity);
	while_entity->setIsUntil(false);

	entity_stack.push(while_entity);
}

template <>
void Listener::exit(BashWhileStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when exiting BashWhileStatement node");
	auto while_entity = std::static_pointer_cast<bpp::IR::BashWhileOrUntilStatement>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashWhileStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(while_entity);
}

template <>
void Listener::enter(BashUntilStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashUntilStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto until_entity = std::make_shared<bpp::IR::BashWhileOrUntilStatement>();
	until_entity->inherit(current_code_entity);
	until_entity->setIsUntil(true);

	entity_stack.push(until_entity);
}

template <>
void Listener::exit(BashUntilStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when exiting BashUntilStatement node");
	auto until_entity = std::static_pointer_cast<bpp::IR::BashWhileOrUntilStatement>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashUntilStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(until_entity);
}

} // namespace bpp::AST
