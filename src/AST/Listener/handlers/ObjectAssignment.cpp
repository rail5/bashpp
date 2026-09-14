/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/ObjectAssignment.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ObjectAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering ObjectAssignment node");
	auto assignment_entity = std::make_shared<bpp::IR::ObjectAssignment>();
	assignment_entity->inherit(entity_stack.top());
	entity_stack.push(assignment_entity);
	context_expectations_stack.push({true, true}); // Lvalue can be either primitive or non-primitive.
}

template <>
void Listener::exit(ObjectAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::ObjectAssignment>(), "Topmost entity is not an ObjectAssignment when exiting ObjectAssignment node");
	auto assignment_entity = std::static_pointer_cast<bpp::IR::ObjectAssignment>(entity_stack.top());
	entity_stack.pop();

	context_expectations_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting ObjectAssignment node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(assignment_entity);
}

} // namespace bpp::AST
