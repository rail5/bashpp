/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ObjectInstantiation.h"

#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/SystemMethod.h>
#include <IR/entities/Object.h>
#include <IR/entities/expressions/bpp/Supershell.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment ObjectInstantiation::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(type != nullptr, "Type pointer is null");

	const auto* object = getStackLikeObject();

	if (object && !object->isPointer()) {
		// Stack-like @TYPE ID instantiation
		return stackLikeInstantiation(state);
	} else {
		// Heap-like call to @new TYPE
		return heapLikeInstantiation(state);
	}
}

bpp::CodeGen::CodeSegment ObjectInstantiation::heapLikeInstantiation(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(type != nullptr, "Type pointer is null");

	bpp::CodeGen::CodeSegment result;

	auto* new_method = type->getMethod_UNSAFE("__new");

	if (state->should_declare_local()) result.add_main_code("local ");
	result.add_main_code("__newAddress=");
	result.egalitarian_merge(Supershell::wrap(state, new_method->getAddress()));
	result.add_main_code("\n");

	result.collapse_to_pre_code();

	result.absorb_all_to_pre(type->callConstructors(state, "${__newAddress}"));

	result.add_main_code("${__newAddress}");

	if (!state->should_declare_local()) result.add_post_code("\nunset __newAddress\n");

	return result;
}

bpp::CodeGen::CodeSegment ObjectInstantiation::stackLikeInstantiation(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(type != nullptr, "Type pointer is null");
	bpp_assert(stackLikeObject != nullptr, "Object pointer is null");

	bpp::CodeGen::CodeSegment result;

	auto* new_method = type->getMethod_UNSAFE("__new");

	bpp_assert(!stackLikeObject->isPrimitive(), "Object is primitive");
	auto requested_address = stackLikeObject->getAddress();

	if (state->should_localize_object_instantiation()) {
		bpp_assert(dynamic_cast<const Builtins::SystemMethod*>(new_method) != nullptr, "Class " + type->getName() + " has a non-SystemMethod __new method");
		const auto* new_sys_method = static_cast<const Builtins::SystemMethod*>(new_method);
		result.egalitarian_merge(new_sys_method->generateInlineNewCode(state, true, stackLikeObject));
	} else {
		result.add_main_code(new_method->getAddress() + " " + requested_address + " >/dev/null\n");
	}

	result.absorb_all_to_main(type->callConstructors(state, requested_address));
	result.add_main_code("bpp____push_objectStack \"" + requested_address + "\"\n");
	state->requires_global_object_stack = true;
	state->requires_vtable_lookup_function = true;

	result.add_main_code("__scopeFrames[-1]=$((${__scopeFrames[-1]} + 1))\n");
	return result;
}

PRETTYPRINT_IMPLEMENTATION(ObjectInstantiation, {
	bpp_assert(type != nullptr, "Type pointer is null");
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ObjectInstantiation ";
	const auto* object = getStackLikeObject();
	if (object && !object->isPointer()) {
		bpp_assert(!object->isPrimitive(), "Object is primitive");
		// Stack-like @TYPE ID instantiation
		os << '@' << type->getName() << ' ' << object->getName();
	} else {
		// Heap-like call to @new TYPE
		os << "@new " << type->getName();
	}
	os << ")\n";
	return os;
});

} // namespace bpp::IR
