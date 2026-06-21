/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/ValueAssignment.h>

#include <IR/entities/DataMember.h>
#include <IR/entities/Object.h>

namespace bpp::AST {

template <>
void Listener::enter(ValueAssignment* node) {
	auto va = std::make_shared<bpp::IR::ValueAssignment>();
	va->set_containing_class(entity_stack.top()->get_containing_class());
	va->inherit(entity_stack.top());

	// FIXME(@rail5): Check object assignment context

	// FIXME(@rail5): Check object instantiation context

	const auto& op = node->OPERATOR();
	va->set_adding(op.getValue() == "+=");

	if (va->is_lvalue_nonprimitive()) {
		context_expectations_stack.push({false, true}); // rvalue must also be nonprimitive
	} else {
		context_expectations_stack.push({true, false}); // rvalue must be primitive
	}

	entity_stack.push(va);
}

template <>
void Listener::exit(ValueAssignment* node) {
	bpp_assert(topmost_entity_is<bpp::IR::ValueAssignment>(), "Topmost entity on stack is not a ValueAssignment when exiting ValueAssignment node");
	auto va = std::static_pointer_cast<bpp::IR::ValueAssignment>(entity_stack.top());
	entity_stack.pop();
	context_expectations_stack.pop();

	auto current_datamember = std::dynamic_pointer_cast<bpp::IR::DataMember>(entity_stack.top());
	if (current_datamember) {
		current_datamember->set_initial_value(va);
		if (va->is_array_assignment()) current_datamember->set_is_array(true);
		return;
	}

	// FIXME(@rail5): Handle object assignment
	// FIXME(@rail5): Handle object instantiations + pointer declarations

	// Default case: just send it up the chain
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting ValueAssignment node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(va);
}

} // namespace bpp::AST
