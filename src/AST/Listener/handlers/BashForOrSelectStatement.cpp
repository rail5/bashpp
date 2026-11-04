/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <error/InternalError.h>

#include <IR/entities/expressions/bash/BashForOrSelectStatement.h>
#include <IR/entities/Program.h>

namespace bpp::AST {

template <>
void Listener::enter(BashForStatement* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto for_statement_entity = std::make_unique<bpp::IR::BashForOrSelectStatement>();
	for_statement_entity->inherit(current_code_entity);
	for_statement_entity->setIsSelect(false);
	for_statement_entity->setVariable(node->VARIABLE().getValue());

	root_program->markEntity(
		getCurrentSourceFile(),
		node->getLine(), node->getCharPositionInLine(),
		node->getEndPosition().line, node->getEndPosition().column,
		for_statement_entity.get()
	);

	entity_stack.push(std::move(for_statement_entity));
}

template <>
void Listener::exit(BashForStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashForOrSelectStatement>(), "Topmost entity is not a BashForOrSelectStatement");
	auto for_statement_entity = entity_stack.pop_as<bpp::IR::BashForOrSelectStatement>();
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(for_statement_entity));
}

template <>
void Listener::enter(BashSelectStatement* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto select_statement_entity = std::make_unique<bpp::IR::BashForOrSelectStatement>();
	select_statement_entity->inherit(current_code_entity);
	select_statement_entity->setIsSelect(true);
	select_statement_entity->setVariable(node->VARIABLE().getValue());

	root_program->markEntity(
		getCurrentSourceFile(),
		node->getLine(), node->getCharPositionInLine(),
		node->getEndPosition().line, node->getEndPosition().column,
		select_statement_entity.get()
	);

	entity_stack.push(std::move(select_statement_entity));
}

template <>
void Listener::exit(BashSelectStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashForOrSelectStatement>(), "Topmost entity is not a BashForOrSelectStatement");
	auto select_statement_entity = entity_stack.pop_as<bpp::IR::BashForOrSelectStatement>();
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(select_statement_entity));
}

template <>
void Listener::enter(BashInCondition* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashForOrSelectStatement>(), "Topmost entity is not a BashForOrSelectStatement");
	auto* for_or_select_statement_entity = entity_stack.top_as<bpp::IR::BashForOrSelectStatement>();

	auto in_condition_entity = std::make_unique<bpp::IR::StringType>();
	in_condition_entity->inherit(for_or_select_statement_entity);

	root_program->markEntity(
		getCurrentSourceFile(),
		node->getLine(), node->getCharPositionInLine(),
		node->getEndPosition().line, node->getEndPosition().column,
		in_condition_entity.get()
	);

	entity_stack.push(std::move(in_condition_entity));
}

template <>
void Listener::exit(BashInCondition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType");
	auto in_condition_entity = entity_stack.pop_as<bpp::IR::StringType>();
	bpp_assert(topmost_entity_is<bpp::IR::BashForOrSelectStatement>(), "Topmost entity is not a BashForOrSelectStatement");
	auto* for_or_select_statement_entity = entity_stack.top_as<bpp::IR::BashForOrSelectStatement>();
	for_or_select_statement_entity->setInCondition(std::move(in_condition_entity));
}

} // namespace bpp::AST
