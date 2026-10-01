/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bpp/ObjectAssignment.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ObjectAssignment* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering ObjectAssignment node");
	auto assignment_entity = std::make_unique<bpp::IR::ObjectAssignment>();
	assignment_entity->inherit(entity_stack.top());
	entity_stack.push(std::move(assignment_entity));
	context_expectations_stack.push({true, true}); // Lvalue can be either primitive or non-primitive.
}

template <>
void Listener::exit(ObjectAssignment* node) {
	bpp_assert(topmost_entity_is<bpp::IR::ObjectAssignment>(), "Topmost entity is not an ObjectAssignment when exiting ObjectAssignment node");
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
		throw bpp::ErrorHandling::SyntaxError(this, value_assignment_ast_node, "Cannot assign a primitive value to a non-primitive object");
	}

	bpp_assert(lhs_is_nonprimitive == rhs_is_nonprimitive, "LHS/RHS primitive/non-primitive mismatch in ObjectAssignment");

	if (lhs_is_nonprimitive && rhs_is_nonprimitive) {
		const auto* lhs = assignment_entity->getLHS()->getReferenceChain().getFinalObject();
		const auto* lhs_type = lhs->getType();
		const auto* rhs = assignment_entity->getRHS()->getRvalueReference()->getReferenceChain().getFinalObject();
		const auto* rhs_type = rhs->getType();
		bpp_assert(lhs_type != nullptr, "LHS type is null in ObjectAssignment");
		bpp_assert(rhs_type != nullptr, "RHS type is null in ObjectAssignment");
		// The RHS type must be the same as or a subclass of the LHS type
		if (rhs_type != lhs_type && !rhs_type->isDerivedFrom(lhs_type)) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Cannot copy object of type '" + rhs_type->getName() + "' to object of type '" + lhs_type->getName() + "'");
		}

		if (rhs_type->isDerivedFrom(lhs_type)) {
			const std::size_t lhs_size = lhs_type->getAllDatamembers().size();
			const std::size_t rhs_size = rhs_type->getAllDatamembers().size();
			const bool at_least_one_object_is_a_pointer = lhs->isPointer() || rhs->isPointer();
			if (rhs_size > lhs_size) {
				const std::size_t slicing_count = rhs_size - lhs_size;
				const std::string_view warning_verb = at_least_one_object_is_a_pointer ? "may result" : "results";
				const std::string_view warning_plural = slicing_count > 1 ? "s" : "";
				const std::string warning_message = std::format(
					"Copying derived class object of type '{}' to base class object of type '{}' "
					"{} in slicing (losing {} data member{})",
					rhs_type->getName(),
					lhs_type->getName(),
					warning_verb,
					slicing_count,
					warning_plural
				);
				show_warning(node, bpp::ErrorHandling::WarningType::Slicing, warning_message);
			}
		}
	}

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting ObjectAssignment node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->add(std::move(assignment_entity));
}

} // namespace bpp::AST
