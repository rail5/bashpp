/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bash/RawSubshell.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(RawSubshell* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering RawSubshell node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto raw_subshell_entity = std::make_unique<bpp::IR::RawSubshell>();
	raw_subshell_entity->inherit(current_code_entity);

	raw_subshell_entity->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	entity_stack.push(std::move(raw_subshell_entity));
}

template <>
void Listener::exit(RawSubshell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::RawSubshell>(), "Topmost entity is not a RawSubshell when exiting RawSubshell node");
	auto raw_subshell_entity = entity_stack.pop_as<bpp::IR::RawSubshell>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting RawSubshell node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(raw_subshell_entity));
}

} // namespace bpp::AST
