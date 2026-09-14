/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ObjectInstantiation.h"

#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/Object.h>
#include <IR/entities/expressions/Supershell.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment ObjectInstantiation::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(!type.expired(), "Type pointer is null");

	auto object = getStackLikeObject().lock();

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
	bpp_assert(!type.expired(), "Type pointer is null");

	bpp::CodeGen::CodeSegment result;

	auto cls = type.lock();
	auto new_method = cls->getMethod_UNSAFE("__new");
	auto constructor = cls->getMethod_UNSAFE("__constructor");

	if (state->should_declare_local()) result.add_main_code("local ");
	result.add_main_code("__newAddress=");
	result.egalitarian_merge(Supershell::wrap(state, new_method->getAddress()));
	result.add_main_code("\n");

	result.collapse_to_pre_code();

	if (constructor) {
		result.add_pre_code(constructor->getAddress() + " ${__newAddress}\n");
	}

	result.add_main_code("${__newAddress}");

	if (!state->should_declare_local()) result.add_post_code("\nunset __newAddress\n");

	return result;
}

bpp::CodeGen::CodeSegment ObjectInstantiation::stackLikeInstantiation(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(!type.expired(), "Type pointer is null");
	bpp_assert(!stackLikeObject.expired(), "Object pointer is null");

	bpp::CodeGen::CodeSegment result;

	auto cls = type.lock();
	auto new_method = cls->getMethod_UNSAFE("__new");
	auto constructor = cls->getMethod_UNSAFE("__constructor");

	auto obj = stackLikeObject.lock();
	bpp_assert(!obj->isPrimitive(), "Object is primitive");
	auto requested_address = obj->getAddress();

	result.add_main_code(new_method->getAddress() + " " + requested_address + " >/dev/null\n");
	if (constructor) {
		result.add_main_code(constructor->getAddress() + " " + requested_address + "\n");
	}

	return result;
}

PRETTYPRINT_IMPLEMENTATION(ObjectInstantiation, {
	bpp_assert(!type.expired(), "Type pointer is null");
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ObjectInstantiation ";
	auto object = getStackLikeObject().lock();
	if (object && !object->isPointer()) {
		bpp_assert(!object->isPrimitive(), "Object is primitive");
		// Stack-like @TYPE ID instantiation
		os << '@' << type.lock()->getName() << ' ' << object->getName();
	} else {
		// Heap-like call to @new TYPE
		os << "@new " << type.lock()->getName();
	}
	os << ")\n";
	return os;
});

} // namespace bpp::IR
