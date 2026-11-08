/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <error/InternalError.h>

#include <IR/entities/expressions/bash/Block.h>
#include <IR/entities/Program.h>
#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/BashFunction.h>
#include <IR/entities/expressions/bash/BashForOrSelectStatement.h>
#include <IR/entities/expressions/bash/BashArithmeticForStatement.h>

namespace bpp::AST {

template <>
void Listener::enter(Block* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	auto block_entity = std::make_unique<bpp::IR::Block>();
	block_entity->inherit(current_code_entity);

	if (source_index) source_index->markContextEntity(
		{getCurrentSourceFile(),
		node->getLine(), node->getColumn(),
		node->getEndPosition().line, node->getEndPosition().column},
		block_entity.get()
	);

	entity_stack.push(std::move(block_entity));
}

template <>
void Listener::exit(Block* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Block>(), "Topmost entity is not a Block");
	auto block_entity = entity_stack.pop_as<bpp::IR::Block>();
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(block_entity));
}

} // namespace bpp::AST
