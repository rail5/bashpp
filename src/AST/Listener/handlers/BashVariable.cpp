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
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto bash_variable_entity = std::make_shared<bpp::IR::StringType>();
	bash_variable_entity->inherit(current_code_entity);
	bash_variable_entity->add("${" + node->TEXT().getValue());
	entity_stack.push(bash_variable_entity);
}

template <>
void Listener::exit(BashVariable* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting BashVariable node");
	auto bash_variable_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	bash_variable_entity->add("}"); // Copy the `}` end token as RawCode into the entity

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting BashVariable node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(bash_variable_entity);
	current_code_entity->adopt_objects_of(bash_variable_entity);
}

} // namespace bpp::AST
