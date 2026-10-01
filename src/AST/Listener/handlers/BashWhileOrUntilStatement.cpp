/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bash/BashWhileOrUntilStatement.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashWhileOrUntilCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when entering BashWhileOrUntilCondition node");
	auto* while_or_until_entity = entity_stack.top_as<bpp::IR::BashWhileOrUntilStatement>();

	auto condition_entity = std::make_unique<bpp::IR::StringType>();
	condition_entity->inherit(while_or_until_entity);

	entity_stack.push(std::move(condition_entity));
}

template <>
void Listener::exit(BashWhileOrUntilCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashWhileOrUntilCondition node");
	auto condition_entity = entity_stack.pop_as<bpp::IR::StringType>();

	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when exiting BashWhileOrUntilCondition node");
	auto* while_or_until_entity = entity_stack.top_as<bpp::IR::BashWhileOrUntilStatement>();

	while_or_until_entity->setCondition(std::move(condition_entity));
}

template <>
void Listener::enter(BashWhileStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashWhileStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto while_entity = std::make_unique<bpp::IR::BashWhileOrUntilStatement>();
	while_entity->inherit(current_code_entity);
	while_entity->setIsUntil(false);

	entity_stack.push(std::move(while_entity));
}

template <>
void Listener::exit(BashWhileStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when exiting BashWhileStatement node");
	auto while_entity = entity_stack.pop_as<bpp::IR::BashWhileOrUntilStatement>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashWhileStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(while_entity));
}

template <>
void Listener::enter(BashUntilStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashUntilStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto until_entity = std::make_unique<bpp::IR::BashWhileOrUntilStatement>();
	until_entity->inherit(current_code_entity);
	until_entity->setIsUntil(true);

	entity_stack.push(std::move(until_entity));
}

template <>
void Listener::exit(BashUntilStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashWhileOrUntilStatement>(), "Topmost entity is not a BashWhileOrUntilStatement when exiting BashUntilStatement node");
	auto until_entity = entity_stack.pop_as<bpp::IR::BashWhileOrUntilStatement>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashUntilStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(until_entity));
}

} // namespace bpp::AST
