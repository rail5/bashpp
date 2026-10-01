/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bpp/DeleteStatement.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(DeleteStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering NewStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto delete_entity = std::make_unique<bpp::IR::DeleteStatement>();
	delete_entity->inherit(current_code_entity);
	entity_stack.push(std::move(delete_entity));

	context_expectations_stack.push({true, false}); // @delete *only* accepts pointers
	// Pointers are primitives, although @delete won't accept "just any" primitive
}

template <>
void Listener::exit(DeleteStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::DeleteStatement>(), "Topmost entity is not a DeleteStatement when exiting DeleteStatement node");
	auto delete_entity = entity_stack.pop_as<bpp::IR::DeleteStatement>();
	context_expectations_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting DeleteStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(delete_entity));
}

} // namespace bpp::AST
