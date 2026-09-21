/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/DeleteStatement.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(DeleteStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering NewStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto delete_entity = std::make_shared<bpp::IR::DeleteStatement>();
	delete_entity->inherit(current_code_entity);
	entity_stack.push(delete_entity);

	context_expectations_stack.push({true, true}); // @delete accepts both pointers and nonprimitives directly
	// Pointers are primitives, although @delete won't accept "just any" primitive
}

template <>
void Listener::exit(DeleteStatement* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::DeleteStatement>(), "Topmost entity is not a DeleteStatement when exiting DeleteStatement node");
	auto delete_entity = std::static_pointer_cast<bpp::IR::DeleteStatement>(entity_stack.top());
	entity_stack.pop();
	context_expectations_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting DeleteStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(delete_entity);
}

} // namespace bpp::AST
