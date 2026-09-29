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
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto string_entity = std::make_unique<bpp::IR::StringType>();
	string_entity->inherit(current_code_entity);
	string_entity->add("\"");
	entity_stack.push(std::move(string_entity));
}

template <>
void Listener::exit(DoublequotedString* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting DoublequotedString node");
	auto string_entity = entity_stack.pop_as<bpp::IR::StringType>();

	string_entity->add("\"");

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting DoublequotedString node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(string_entity));
}

} // namespace bpp::AST
