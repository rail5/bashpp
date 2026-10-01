/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "MethodParameter.h"

#include <IR/entities/Object.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/Class.h>
#include <IR/entities/expressions/bpp/DynamicCast.h>
#include <IR/entities/Method.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment MethodParameter::generateCode(bpp::CodeGen::CodeGenState* /*state*/) const {
	throw bpp::ErrorHandling::InternalError("MethodParameter::generateCode() called without index parameter");
}

bpp::CodeGen::CodeSegment MethodParameter::generateCode(bpp::CodeGen::CodeGenState* state, std::uint32_t index) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment code;

	bpp_assert(getType() == nullptr || isPointer(), "MethodParameter is neither a pointer nor a primitive type");

	if (getType() == nullptr) {
		// Primitive type: retrieve the value from the corresponding positional parameter
		code.add_main_code("local " + name + "=\"$" + std::to_string(index) + "\"\n");
	} else {
		// Pointer to nonprimitive: per the Bash++ spec, this is an implicit dynamic cast to the expected type
		// I.e., retrieve the corresponding positional parameter, run it through a dynamic cast, and assign the result to this pointer
		//
		// We expect that the `initial_value` should have already been set to a DynamicCast
		bpp_assert(isPointer(), "MethodParameter is not a pointer but has a nonprimitive type");
		bpp_assert(hasInitialValue(), "MethodParameter has a nonprimitive type but no initial value set");
		bpp_assert(dynamic_cast<const DynamicCast*>(getInitialValue().value()), "MethodParameter has a nonprimitive type but its initial value is not a DynamicCast");

		if (state->should_declare_local()) code.add_main_code("local ");
		code.add_main_code(getAddress() + "\n");
		const auto* initial_value = static_cast<const DynamicCast*>(getInitialValue().value());
		code.add_main_code(initial_value->generateCode(state, getAddress()).get_pre_code());
		code.add_main_code("\n");
	}
	return code;
}

ThisPtr::ThisPtr(const Class* containing_class) {
	setName("this");
	setType(containing_class);
	setIsPointer(true);
}

bpp::CodeGen::CodeSegment ThisPtr::generateCode(bpp::CodeGen::CodeGenState* state, std::uint32_t /*index*/) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(hasInitialValue(), "ThisPtr has no initial value set");
	bpp_assert(state->in_method(), "Must be generating code for a method");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("local __this\n");

	bpp_assert(dynamic_cast<const DynamicCast*>(getInitialValue().value()), "The initial value of the implicit `this` parameter is not a DynamicCast entity");
	const auto* dynamic_cast_entity = static_cast<const DynamicCast*>(getInitialValue().value());

	// A dynamic cast of $1 to the expected type, with the result assigned to `this`.
	code.add_pre_code("if ! ");
	code.add_pre_code(dynamic_cast_entity->generateCode(state, "__this").get_pre_code());
	code.add_pre_code("then\n"
	"\t>&2 echo \"Bash++: Error: Attempted to call @"
	+ getType()->getName() + "." + state->current_method->getName()
		+ " on null object\"\n"
		"\treturn 1\n"
		"fi\n"
	);

	code.add_post_code("shift 1\n"); // Shift the positional parameters to remove the `this` argument

	return code;
}

RequestedAddressParam::RequestedAddressParam(const Class* containing_class) : ThisPtr(containing_class) {}

bpp::CodeGen::CodeSegment RequestedAddressParam::generateCode(bpp::CodeGen::CodeGenState* state, std::uint32_t /*index*/) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(state->in_method(), "Must be generating code for a method");

	const auto* cls = getContainingClass();
	bpp_assert(cls != nullptr, "Containing class is null");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("local __this=$1\n");

	code.add_pre_code(R"EOF(if [[ -z "${__this}" ]]; then
	while : ; do
		__this="bpp__)EOF" + cls->getName() + R"EOF(__$RANDOM$RANDOM$RANDOM$RANDOM"
		local __vpVar="${__this}____vPointer"
		[[ -z "${!__vpVar+x}" ]] && break
	done
fi
)EOF");

	code.add_post_code("shift 1\n"); // Shift the positional parameters to remove the `this` argument

	return code;
}

CopyFromParam::CopyFromParam(const Class* containing_class) : ThisPtr(containing_class) {
	setName("source");
}

bpp::CodeGen::CodeSegment CopyFromParam::generateCode(bpp::CodeGen::CodeGenState* state, std::uint32_t /*index*/) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(hasInitialValue(), "CopyFromParam has no initial value set");
	bpp_assert(state->in_method(), "Must be generating code for a method");
	bpp_assert(state->current_method->viewName() == "__copy", "CopyFromParam can only be used in the __copy method");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("local __source\n");

	bpp_assert(dynamic_cast<const DynamicCast*>(getInitialValue().value()), "The initial value of the implicit `source` parameter is not a DynamicCast entity");
	const auto* dynamic_cast_entity = static_cast<const DynamicCast*>(getInitialValue().value());

	code.add_pre_code("if ! ");
	code.add_pre_code(dynamic_cast_entity->generateCode(state, "__source").get_pre_code());
	code.add_pre_code("then\n"
	"\t>&2 echo \"Bash++: Error: Class " + getType()->getName() + ": Attempted to copy from null object or object of incompatible type\"\n"
		"\treturn 1\n"
		"fi\n"
	);

	return code;
}

} // namespace bpp::IR
