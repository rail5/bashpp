/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Method.h>
#include <IR/entities/expressions/ObjectReference.h>
#include <IR/entities/expressions/Supershell.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

#include <span>

namespace bpp::AST {

template <>
void Listener::enter(ObjectReference* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "ObjectReference node must be inside a code entity");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	/*
	 * There are 12 possible combinations of flags here:
	 *      LVALUE  SELF_REF    POINTER_DEREF   OBJ_ADDRESS   Meaning                                   Example
	 * 1.   0       0           0               0             Object reference as rvalue                echo @obj.member
	 * 2.   0       0           0               1             Object address as rvalue                  echo &@obj.member
	 * 3.   0       0           1               0             Pointer dereference as rvalue             echo *@obj.member
	 * 4.   0       1           0               0             Self reference as rvalue                  echo @this.member
	 * 5.   0       1           0               1             Self reference as rvalue (address of)     echo &@this.member
	 * 6.   0       1           1               0             Self reference as rvalue (pointer deref)  echo *@this.member
	 * 7.   1       0           0               0             Object reference as lvalue                @obj.member arg1 arg2
	 * 8.   1       0           0               1             Object address as lvalue                  &@obj.member arg1 arg2
	 * 9.   1       0           1               0             Pointer dereference as lvalue             *@obj.member arg1 arg2
	 * 10.  1       1           0               0             Self reference as lvalue                  @this.member arg1 arg2
	 * 11.  1       1           0               1             Self reference as lvalue (address of)     &@this.member arg1 arg2
	 * 12.  1       1           1               0             Self reference as lvalue (pointer deref)  *@this.member arg1 arg2
	 *
	 * POINTER_DEREF and OBJ_ADDRESS are mutually exclusive, so combinations where both are 1 are invalid
	 *
	 */
	
	bpp_assert(!(node->isPointerDereference() && node->isAddressOf()), "Detected simultaneous pointer dereference and object address");

	auto resolution = bpp::IR::resolve_entity(
		get_current_source_file(),
		current_code_entity,
		std::span{node->IDENTIFIERS()}
	);

	if (!resolution) {
		const auto& error = resolution.error();
		if (error.token.has_value()) {
			throw bpp::ErrorHandling::SyntaxError(this, error.token.value(), error.message);
		} else {
			throw bpp::ErrorHandling::SyntaxError(this, node, error.message);
		}
	}

	const auto& reference_entity = resolution.value();

	const bool is_nonprimitive_reference = [&node, &reference_entity]() {
		if (node->isAddressOf()) return false;

		if (reference_entity->isNonprimitive()) return true;

		if (reference_entity->isPointer() && node->isPointerDereference()) return true;

		return false;
	}();

	if (is_nonprimitive_reference && !context_expectations_stack.canTakeObject()) {
		// Non-primitive referenced where a primitive is expected: implicit call to .toPrimitive
		reference_entity->addToPrimitiveCall();
	}

	reference_entity->inherit(current_code_entity);
	reference_entity->setLvalue(node->isLvalue());
	reference_entity->setAddressOf(node->isAddressOf());
	entity_stack.push(reference_entity);
}

template <>
void Listener::exit(ObjectReference* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::ObjectReference>(), "Topmost entity on stack is not an ObjectReference when exiting ObjectReference node");
	auto reference_entity = std::static_pointer_cast<bpp::IR::ObjectReference>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "ObjectReference node must be inside a code entity");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(reference_entity);
}

} // namespace bpp::AST
