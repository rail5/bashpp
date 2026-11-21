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

	if (lhs_is_nonprimitive) {
		auto lhs = assignment_entity->releaseLHS();
		auto rhs = assignment_entity->releaseRHS()->releaseRvalueReference();
		EntityFabricator::makeNonprimitiveCopy(this, node, std::move(lhs), std::move(rhs));
		// makeNonPrimitiveCopy() errors if types are incompatible or rvalue is primitive
		// so we don't need to do any further validation here
		return;
	}

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(assignment_entity));
}

} // namespace bpp::AST
