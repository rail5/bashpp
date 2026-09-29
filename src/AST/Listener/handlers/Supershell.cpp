/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/Supershell.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(Supershell* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering Supershell node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto supershell_entity = std::make_unique<bpp::IR::Supershell>();
	supershell_entity->inherit(current_code_entity);

	supershell_entity->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	entity_stack.push(std::move(supershell_entity));
}

template <>
void Listener::exit(Supershell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Supershell>(), "Topmost entity is not a Supershell when exiting Supershell node");
	auto supershell_entity = entity_stack.pop_as<bpp::IR::Supershell>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting Supershell node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->adoptObjectsOf(supershell_entity.get());
	current_code_entity->add(std::move(supershell_entity));
}

} // namespace bpp::AST
