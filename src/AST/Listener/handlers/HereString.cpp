/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>
#include <error/InternalError.h>

#include <IR/entities/expressions/String.h>

namespace bpp::AST {

template <>
void Listener::enter(HereString* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto here_string_entity = std::make_unique<bpp::IR::StringType>();
	here_string_entity->inherit(current_code_entity);
	here_string_entity->add("<<<");
	entity_stack.push(std::move(here_string_entity));
}

template <>
void Listener::exit(HereString* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType");
	auto here_string_entity = entity_stack.pop_as<bpp::IR::StringType>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(here_string_entity));
}

} // namespace bpp::AST
