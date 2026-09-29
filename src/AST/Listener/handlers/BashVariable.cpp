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
void Listener::enter(BashVariable* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering BashRedirection node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto bash_variable_entity = std::make_unique<bpp::IR::StringType>();
	bash_variable_entity->inherit(current_code_entity);
	bash_variable_entity->add("${" + node->TEXT().getValue());
	entity_stack.push(std::move(bash_variable_entity));
}

template <>
void Listener::exit(BashVariable* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashVariable node");
	auto bash_variable_entity = entity_stack.pop_as<bpp::IR::StringType>();

	bash_variable_entity->add("}"); // Copy the `}` end token as RawCode into the entity

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashVariable node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	
	current_code_entity->adoptObjectsOf(bash_variable_entity.get());
	current_code_entity->add(std::move(bash_variable_entity));
}

} // namespace bpp::AST
