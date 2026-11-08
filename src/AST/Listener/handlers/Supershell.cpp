/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bpp/Supershell.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(Supershell* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto supershell_entity = std::make_unique<bpp::IR::Supershell>();
	supershell_entity->inherit(current_code_entity);

	if (source_index) source_index->markContextEntity(
		{getCurrentSourceFile(),
		node->getLine(), node->getColumn(),
		node->getEndPosition().line, node->getEndPosition().column},
		supershell_entity.get()
	);

	entity_stack.push(std::move(supershell_entity));
}

template <>
void Listener::exit(Supershell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Supershell>(), "Topmost entity is not a Supershell");
	auto supershell_entity = entity_stack.pop_as<bpp::IR::Supershell>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->adoptObjectsOf(supershell_entity.get());
	current_code_entity->add(std::move(supershell_entity));
}

} // namespace bpp::AST
