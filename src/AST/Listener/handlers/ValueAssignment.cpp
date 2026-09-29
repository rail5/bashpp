/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/ValueAssignment.h>

#include <IR/entities/DataMember.h>
#include <IR/entities/Object.h>
#include <IR/entities/expressions/ObjectAssignment.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ValueAssignment* node) {
	auto va = std::make_unique<bpp::IR::ValueAssignment>();
	va->inherit(entity_stack.top());

	if (auto* current_object_assignment = dynamic_cast<bpp::IR::ObjectAssignment*>(entity_stack.top())) {
		va->setLvalueNonprimitive(current_object_assignment->getLHS()->isNonprimitive());
	}

	if (auto* current_object_instantiation = dynamic_cast<bpp::IR::Object*>(entity_stack.top())) {
		va->setLvalueNonprimitive(!current_object_instantiation->isPrimitive());
	}

	const auto& op = node->OPERATOR();
	va->setAdding(op.getValue() == "+=");

	if (va->isLvalueNonprimitive()) {
		context_expectations_stack.push({false, true}); // rvalue must also be nonprimitive
	} else {
		context_expectations_stack.push({true, false}); // rvalue must be primitive
	}

	entity_stack.push(std::move(va));
}

template <>
void Listener::exit(ValueAssignment* node) {
	bpp_assert(topmost_entity_is<bpp::IR::ValueAssignment>(), "Topmost entity on stack is not a ValueAssignment when exiting ValueAssignment node");
	auto va = entity_stack.pop_as<bpp::IR::ValueAssignment>();
	context_expectations_stack.pop();

	bpp_assert(va->isLvalueNonprimitive() || !va->isRvalueNonprimitive(), "Compiler attempted to assign a non-primitive value to a primitive variable");

	if (auto* current_datamember = dynamic_cast<bpp::IR::DataMember*>(entity_stack.top())) {
		if (va->isArrayAssignment()) current_datamember->setIsArray(true);
		current_datamember->setInitialValue(std::move(va));
		return;
	}

	if (auto* current_object_assignment = dynamic_cast<bpp::IR::ObjectAssignment*>(entity_stack.top())) {
		current_object_assignment->setRHS(std::move(va));
		return;
	}

	if (auto* current_object = dynamic_cast<bpp::IR::Object*>(entity_stack.top())) {
		// FIXME(@rail5): This only handles the pointer case, handle the non-pointer case
		if (va->isArrayAssignment()) {
			throw bpp::ErrorHandling::SyntaxError(this, node, "Cannot assign an array to an object");
		}
		current_object->setInitialValue(std::move(va));
		return;
	}

	// Default case: just send it up the chain
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting ValueAssignment node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(va));
}

} // namespace bpp::AST
