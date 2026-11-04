/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <error/InternalError.h>

#include <IR/entities/expressions/bash/Block.h>
#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/BashFunction.h>
#include <IR/entities/expressions/bash/BashForOrSelectStatement.h>
#include <IR/entities/expressions/bash/BashArithmeticForStatement.h>

namespace bpp::AST {

namespace {

bool is_surrounded_by_larger_construct(const IR::EntityStack& entity_stack) {
	const bool for_class = entity_stack.top_is<bpp::IR::Class>();
	const bool for_method = entity_stack.top_is<bpp::IR::Method>();
	const bool for_function = entity_stack.top_is<bpp::IR::BashFunction>();
	const bool for_for_or_select = entity_stack.top_is<bpp::IR::BashForOrSelectStatement>() || entity_stack.top_is<bpp::IR::BashArithmeticForStatement>();

	return for_class || for_method || for_function || for_for_or_select;
}

} // anonymous namespace

template <>
void Listener::enter(Block* /*node*/) {
	if (is_surrounded_by_larger_construct(entity_stack)) return; // Block creation handled by the surrounding construct

	// Otherwise:
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	auto block_entity = std::make_unique<bpp::IR::Block>();
	block_entity->inherit(current_code_entity);
	entity_stack.push(std::move(block_entity));
}

template <>
void Listener::exit(Block* /*node*/) {
	if (is_surrounded_by_larger_construct(entity_stack)) return; // Block creation handled by the surrounding construct

	bpp_assert(topmost_entity_is<bpp::IR::Block>(), "Topmost entity is not a Block");
	auto block_entity = entity_stack.pop_as<bpp::IR::Block>();
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(block_entity));
}

} // namespace bpp::AST
