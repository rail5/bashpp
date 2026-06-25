/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/Supershell.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(Supershell* node) {
	auto current_code_entity = std::dynamic_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	if (!current_code_entity) throw bpp::ErrorHandling::InternalError("Supershell outside of a code entity");

	auto supershell_entity = std::make_shared<bpp::IR::Supershell>();
	supershell_entity->set_containing_class(current_code_entity->get_containing_class().lock());
	supershell_entity->inherit(current_code_entity);
	entity_stack.push(supershell_entity);

	supershell_entity->set_definition_position({
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	});
}

template <>
void Listener::exit(Supershell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Supershell>(), "Topmost entity is not a Supershell when exiting Supershell node");
	auto supershell_entity = std::static_pointer_cast<bpp::IR::Supershell>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting Supershell node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(supershell_entity);
	current_code_entity->adopt_objects_of(supershell_entity);
}

} // namespace bpp::AST
