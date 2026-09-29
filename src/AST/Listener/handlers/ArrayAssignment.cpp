/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ArrayAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto array_index_entity = std::make_unique<bpp::IR::StringType>();
	array_index_entity->inherit(current_code_entity);
	array_index_entity->add("(");

	entity_stack.push(std::move(array_index_entity));
}

template <>
void Listener::exit(ArrayAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType");
	auto array_index_entity = entity_stack.pop_as<bpp::IR::StringType>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	array_index_entity->add(")");
	current_code_entity->add(std::move(array_index_entity));
}

} // namespace bpp::AST
