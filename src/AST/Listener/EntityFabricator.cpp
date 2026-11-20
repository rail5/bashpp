/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "EntityFabricator.h"

#include <IR/entities/expressions/bpp/ObjectReference.h>
#include <IR/entities/Class.h>
#include <IR/entities/CodeEntity.h>

#include <memory>
#include <string_view>
#include <format>

namespace bpp::AST {

void Listener::EntityFabricator::makeNonprimitiveCopy(
	Listener* listener,
	ASTNode* node,
	std::unique_ptr<bpp::IR::ObjectReference> lhs,
	std::unique_ptr<bpp::IR::ObjectReference> rhs
) {
	bpp_assert(listener != nullptr, "Listener pointer is null");
	bpp_assert(node != nullptr, "ASTNode pointer is null");
	bpp_assert(lhs != nullptr, "LHS ObjectReference pointer is null");
	bpp_assert(rhs != nullptr, "RHS ObjectReference pointer is null");

	bpp_assert(lhs->isNonprimitive(), "LHS ObjectReference is not non-primitive");
	if (!rhs->isNonprimitive()) listener->error(node, "Cannot assign a primitive value to a non-primitive object");

	const auto* lhs_final_object = lhs->getReferenceChain().getFinalObject();
	const auto* rhs_final_object = rhs->getReferenceChain().getFinalObject();
	bpp_assert(lhs_final_object != nullptr, "LHS final object is null");
	bpp_assert(rhs_final_object != nullptr, "RHS final object is null");

	const auto* lhs_type = lhs_final_object->getType();
	const auto* rhs_type = rhs_final_object->getType();
	bpp_assert(lhs_type != nullptr, "LHS type is null");
	bpp_assert(rhs_type != nullptr, "RHS type is null");

	bpp_assert(listener->topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = listener->entity_stack.top_as<bpp::IR::CodeEntity>();

	// The RHS type must be the same as or a subclass of the LHS type
	if (rhs_type != lhs_type && !rhs_type->isDerivedFrom(lhs_type)) {
		listener->error(node, "Cannot copy object of type '" + rhs_type->getName() + "' to object of type '" + lhs_type->getName() + "'");
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
				"Copying object of type '{}' to object of type '{}' "
				"{} in slicing (losing {} data member{})",
				rhs_type->getName(),
				lhs_type->getName(),
				warning_verb,
				slicing_count,
				warning_plural
			);
			listener->warn(node, bpp::ErrorHandling::WarningType::Slicing, warning_message);
		}
	}


	// Synthetically create the entities necessary for the nonprimitive copy:
	// 1. A method call to @lhs.__copy
	// 2. An object reference to the RHS, with the address-of operator applied
	lhs->addMethodCall_UNSAFE("__copy");
	lhs->setLvalue(true);
	lhs->reparent(current_code_entity);

	rhs->setLvalue(false);
	rhs->setAddressOf(true);
	rhs->reparent(current_code_entity);

	current_code_entity->add(std::move(lhs));
	current_code_entity->add(" ");
	current_code_entity->add(std::move(rhs));
	current_code_entity->add("\n");
}

} // namespace bpp::AST
