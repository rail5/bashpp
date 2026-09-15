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
void Listener::exit(ObjectAssignment* node) {
	bpp_assert(topmost_entity_is<bpp::IR::ObjectAssignment>(), "Topmost entity is not an ObjectAssignment when exiting ObjectAssignment node");
	auto assignment_entity = std::static_pointer_cast<bpp::IR::ObjectAssignment>(entity_stack.top());
	entity_stack.pop();

	context_expectations_stack.pop();

	const bool lhs_is_nonprimitive = assignment_entity->getLHS()->isNonprimitive();
	const bool rhs_is_nonprimitive = assignment_entity->getRHS()->isRvalueNonprimitive();
	bpp_assert(lhs_is_nonprimitive == rhs_is_nonprimitive, "LHS/RHS primitive/non-primitive mismatch in ObjectAssignment");

	if (lhs_is_nonprimitive && rhs_is_nonprimitive) {
		auto lhs_type = assignment_entity->getLHS()->getReferenceChain().getFinalObject().lock()->getType().lock();
		auto rhs_type = assignment_entity->getRHS()->getRvalueObject()->getReferenceChain().getFinalObject().lock()->getType().lock();
		bpp_assert(lhs_type != nullptr, "LHS type is null in ObjectAssignment");
		bpp_assert(rhs_type != nullptr, "RHS type is null in ObjectAssignment");
		// The RHS type must be the same as or a subclass of the LHS type
		if (!(rhs_type == lhs_type || rhs_type->isDerivedFrom(lhs_type))) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Cannot copy object of type '" + rhs_type->getName() + "' to object of type '" + lhs_type->getName() + "'");
		}

		if (rhs_type->isDerivedFrom(lhs_type)) {
			const std::size_t lhs_size = lhs_type->getAllDatamembers().size();
			const std::size_t rhs_size = rhs_type->getAllDatamembers().size();
			if (rhs_size > lhs_size) {
				const std::size_t slicing_count = rhs_size - lhs_size;
				std::string warning_message = "Copying derived class object of type '";
				warning_message += rhs_type->getName();
				warning_message += "' to base class object of type '";
				warning_message += lhs_type->getName();
				warning_message += "' results in slicing (losing ";
				warning_message += std::to_string(slicing_count);
				warning_message += " data member";
				if (slicing_count > 1) warning_message += 's';
				warning_message += ')';
				show_warning(node, bpp::ErrorHandling::WarningType::Slicing, warning_message);
			}
		}
		// Mark the LHS class's __copy method as referenced by this assignment
		auto copy_method = lhs_type->getMethod_UNSAFE("__copy");
		bpp_assert(copy_method != nullptr, "LHS class has no __copy method in ObjectAssignment");
		copy_method->markReferencedBy(assignment_entity);
	}

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting ObjectAssignment node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(assignment_entity);
}

} // namespace bpp::AST
