/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bash/BashCaseStatement.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashCaseStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity");
	auto* current_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	auto case_statement_entity = std::make_unique<bpp::IR::BashCaseStatement>();
	case_statement_entity->inherit(current_entity);
	entity_stack.push(std::move(case_statement_entity));
}

template <>
void Listener::exit(BashCaseStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashCaseStatement>(), "Topmost entity on stack is not a BashCaseStatement");
	auto case_statement_entity = entity_stack.pop_as<bpp::IR::BashCaseStatement>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity");
	auto* current_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_entity->add(std::move(case_statement_entity));
}

template <>
void Listener::enter(BashCaseInput* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashCaseStatement>(), "Topmost entity on stack is not a BashCaseStatement");
	auto* case_statement_entity = entity_stack.top_as<bpp::IR::BashCaseStatement>();
	auto case_input_entity = std::make_unique<bpp::IR::StringType>();
	case_input_entity->inherit(case_statement_entity);
	entity_stack.push(std::move(case_input_entity));
}

template <>
void Listener::exit(BashCaseInput* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity on stack is not a StringType");
	auto case_input_entity = entity_stack.pop_as<bpp::IR::StringType>();
	bpp_assert(topmost_entity_is<bpp::IR::BashCaseStatement>(), "Topmost entity on stack is not a BashCaseStatement");
	auto* case_statement_entity = entity_stack.top_as<bpp::IR::BashCaseStatement>();
	case_statement_entity->setCaseInput(std::move(case_input_entity));
}

template <>
void Listener::enter(BashCasePattern* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashCaseStatement>(), "Topmost entity on stack is not a BashCaseStatement");
	auto* case_statement_entity = entity_stack.top_as<bpp::IR::BashCaseStatement>();
	auto case_pattern_entity = std::make_unique<bpp::IR::BashCasePattern>();
	case_pattern_entity->inherit(case_statement_entity);
	entity_stack.push(std::move(case_pattern_entity));
}

template <>
void Listener::exit(BashCasePattern* node) {
	bpp_assert(topmost_entity_is<bpp::IR::BashCasePattern>(), "Topmost entity on stack is not a BashCasePattern");
	auto case_pattern_entity = entity_stack.pop_as<bpp::IR::BashCasePattern>();
	case_pattern_entity->add("\n" + node->TERMINATOR().getValue() + "\n");

	bpp_assert(topmost_entity_is<bpp::IR::BashCaseStatement>(), "Topmost entity on stack is not a BashCaseStatement");
	auto* case_statement_entity = entity_stack.top_as<bpp::IR::BashCaseStatement>();
	case_statement_entity->add(std::move(case_pattern_entity));
}

template <>
void Listener::enter(BashCasePatternHeader* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashCasePattern>(), "Topmost entity on stack is not a BashCasePattern");
	auto* case_pattern_entity = entity_stack.top_as<bpp::IR::BashCasePattern>();
	auto pattern_header_entity = std::make_unique<bpp::IR::StringType>();
	pattern_header_entity->inherit(case_pattern_entity);
	entity_stack.push(std::move(pattern_header_entity));
}

template <>
void Listener::exit(BashCasePatternHeader* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity on stack is not a StringType");
	auto pattern_header_entity = entity_stack.pop_as<bpp::IR::StringType>();
	bpp_assert(topmost_entity_is<bpp::IR::BashCasePattern>(), "Topmost entity on stack is not a BashCasePattern");
	auto* case_pattern_entity = entity_stack.top_as<bpp::IR::BashCasePattern>();
	case_pattern_entity->setPatternHeader(std::move(pattern_header_entity));
}

} // namespace bpp::AST
