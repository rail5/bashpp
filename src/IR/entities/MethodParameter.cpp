/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "MethodParameter.h"

#include <IR/entities/Object.h>
#include <IR/entities/expressions/DynamicCast.h>
#include <IR/entities/Method.h>

#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment MethodParameter::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "MethodParameter::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	bpp_assert(type.expired() || m_is_pointer, "MethodParameter is neither a pointer nor a primitive type");

	if (type.expired()) {
		// Primitive type: retrieve the value from the corresponding positional parameter
		code.add_main_code("local " + name + "=\"$" + std::to_string(index) + "\"\n");
	} else {
		// Pointer to nonprimitive: per the Bash++ spec, this is an implicit dynamic cast to the expected type
		// I.e., retrieve the corresponding positional parameter, run it through a dynamic cast, and assign the result to this pointer
		//
		// We expect that the `initial_value` should have already been set to a DynamicCast
		bpp_assert(is_pointer(), "MethodParameter is not a pointer but has a nonprimitive type");
		bpp_assert(has_initial_value(), "MethodParameter has a nonprimitive type but no initial value set");
		bpp_assert(std::dynamic_pointer_cast<DynamicCast>(initial_value.value()), "MethodParameter has a nonprimitive type but its initial value is not a DynamicCast");

		if (state->should_declare_local()) code.add_main_code("local ");
		code.add_main_code(get_address() + "=");
		code.egalitarian_merge(initial_value.value()->generate_code(state));
		code.add_main_code("\n");
	}
	return code;
}

bpp::CodeGen::CodeSegment ThisPtr::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "ThisPtr::generate_code() should be called with a non-null state pointer");
	bpp_assert(has_initial_value(), "ThisPtr has no initial value set");
	bpp_assert(state->in_method(), "ThisPtr::generate_code() should only be called when generating code for a method");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("local __this\n");

	auto dynamic_cast_entity = std::dynamic_pointer_cast<DynamicCast>(initial_value.value());
	if (!dynamic_cast_entity) {
		throw bpp::ErrorHandling::InternalError("The initial value of the implicit `this` parameter is not a DynamicCast entity");
	}
	dynamic_cast_entity->set_target_variable("__this");

	// A dynamic cast of $1 to the expected type, with the result assigned to `this`.
	code.add_pre_code("if ! ");
	code.add_pre_code(dynamic_cast_entity->generate_code(state).get_pre_code());
	code.add_pre_code("then\n"
	"\t>&2 echo \"Bash++: Error: Attempted to call @"
	+ type.lock()->get_name() + "." + state->current_method->get_name()
		+ " on null object\"\n"
		"\treturn 1\n"
		"fi\n"
	);

	code.add_post_code("shift 1\n"); // Shift the positional parameters to remove the `this` argument

	return code;
}

	return code;
}

} // namespace bpp::IR
