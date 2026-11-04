/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bash/RawSubshell.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(RawSubshell* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto raw_subshell_entity = std::make_unique<bpp::IR::RawSubshell>();
	raw_subshell_entity->inherit(current_code_entity);

	root_program->markEntity(
		getCurrentSourceFile(),
		node->getLine(), node->getCharPositionInLine(),
		node->getEndPosition().line, node->getEndPosition().column,
		raw_subshell_entity.get()
	);

	entity_stack.push(std::move(raw_subshell_entity));
}

template <>
void Listener::exit(RawSubshell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::RawSubshell>(), "Topmost entity is not a RawSubshell");
	auto raw_subshell_entity = entity_stack.pop_as<bpp::IR::RawSubshell>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(raw_subshell_entity));
}

} // namespace bpp::AST
