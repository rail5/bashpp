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
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto supershell_entity = std::make_shared<bpp::IR::Supershell>();
	supershell_entity->inherit(current_code_entity);
	entity_stack.push(supershell_entity);

	supershell_entity->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	auto containing_program = current_code_entity->getContainingProgram();
	bpp_assert(!containing_program.expired(), "Containing program is null when entering Supershell node");
	auto supershell_builtin = containing_program.lock()->getSupershellFunction();
	bpp_assert(supershell_builtin != nullptr, "Supershell builtin function is null when entering Supershell node");
	supershell_builtin->markReferencedBy(supershell_entity);
}

template <>
void Listener::exit(Supershell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Supershell>(), "Topmost entity is not a Supershell when exiting Supershell node");
	auto supershell_entity = std::static_pointer_cast<bpp::IR::Supershell>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting Supershell node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(supershell_entity);
	current_code_entity->adoptObjectsOf(supershell_entity);
}

} // namespace bpp::AST
