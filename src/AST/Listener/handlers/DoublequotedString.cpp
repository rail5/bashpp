/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(DoublequotedString* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering DoublequotedString node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto string_entity = std::make_shared<bpp::IR::String>();
	string_entity->set_containing_class(current_code_entity->get_containing_class().lock());
	string_entity->inherit(current_code_entity);
	entity_stack.push(string_entity);
}

template <>
void Listener::exit(DoublequotedString* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::String>(), "Topmost entity is not a String when exiting DoublequotedString node");
	auto string_entity = std::static_pointer_cast<bpp::IR::String>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting DoublequotedString node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(string_entity);
}

} // namespace bpp::AST
