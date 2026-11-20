/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>
#include <IR/entities/expressions/bpp/ObjectAssignment.h>
#include <AST/Listener/EntityFabricator.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(ObjectAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto assignment_entity = std::make_unique<bpp::IR::ObjectAssignment>();
	assignment_entity->inherit(entity_stack.top());
	entity_stack.push(std::move(assignment_entity));
	context_expectations_stack.push({true, true}); // Lvalue can be either primitive or non-primitive.
}

template <>
void Listener::exit(ObjectAssignment* node) {
	bpp_assert(topmost_entity_is<bpp::IR::ObjectAssignment>(), "Topmost entity is not an ObjectAssignment");
	auto assignment_entity = entity_stack.pop_as<bpp::IR::ObjectAssignment>();
	context_expectations_stack.pop();

	// If one of the inner handlers bailed out early due to an error,
	//  (e.g., the LHS or RHS were invalid)
	// then we should also bail out early here (don't duplicate the same error messages, the user's already seen them)
	if (!assignment_entity->getLHS() || !assignment_entity->getRHS()) return;

	const bool lhs_is_nonprimitive = assignment_entity->getLHS()->isNonprimitive();
	const bool rhs_is_nonprimitive = assignment_entity->getRHS()->isRvalueNonprimitive();

	if (lhs_is_nonprimitive && !rhs_is_nonprimitive) {
		const auto* value_assignment_ast_node = node->getLastChild(); // FIXME(@rail5): Brittle
		error(value_assignment_ast_node, "Cannot assign a primitive value to a non-primitive object");
	}

	bpp_assert(lhs_is_nonprimitive == rhs_is_nonprimitive, "LHS/RHS primitive/non-primitive mismatch in ObjectAssignment");

	if (lhs_is_nonprimitive && rhs_is_nonprimitive) {
		auto lhs = assignment_entity->releaseLHS();
		auto rhs = assignment_entity->releaseRHS()->releaseRvalueReference();
		EntityFabricator::makeNonprimitiveCopy(this, node, std::move(lhs), std::move(rhs));
		return;
	}

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(assignment_entity));
}

} // namespace bpp::AST
