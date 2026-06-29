/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/RawSubshell.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(RawSubshell* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering RawSubshell node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto raw_subshell_entity = std::make_shared<bpp::IR::RawSubshell>();
	raw_subshell_entity->set_containing_class(current_code_entity->get_containing_class().lock());
	raw_subshell_entity->inherit(current_code_entity);
	entity_stack.push(raw_subshell_entity);

	raw_subshell_entity->set_definition_position({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});
}

template <>
void Listener::exit(RawSubshell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::RawSubshell>(), "Topmost entity is not a RawSubshell when exiting RawSubshell node");
	auto raw_subshell_entity = std::static_pointer_cast<bpp::IR::RawSubshell>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting RawSubshell node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(raw_subshell_entity);
}

} // namespace bpp::AST
