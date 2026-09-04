/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include <IR/entities/DataMember.h>

#include <IR/entities/expressions/Supershell.h>
#include <IR/entities/expressions/ObjectReference.h>

#include "SystemMethod.h"

#include <error/InternalError.h>

namespace bpp::IR::Builtins {

bpp::CodeGen::CodeSegment SystemMethod::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "SystemMethod::generate_code() should be called with a non-null state pointer");
	state->current_method = shared_from_this();

	bpp::CodeGen::CodeSegment result;

	result.add_pre_code(getAddress() + "() {\n");

	for (const auto& param : getParameters()) {
		result.absorb_all_to_pre(param->generateCode(state));
	}

	result.egalitarian_merge(generateInlineCode(state, false));

	// __new should echo the address of the new object, so that it can be captured by the caller
	if (type == Type::NEW) result.add_main_code("\necho ${__this}\n");

	result.add_post_code("}\n");

	state->current_method = nullptr;
	return result;
}

bpp::CodeGen::CodeSegment SystemMethod::generateInlineCode(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const {
	bpp_assert(state != nullptr, "SystemMethod::generate_inline_code() should be called with a non-null state pointer");

	if (!obj) {
		auto cls = getContainingClass().lock();
		bpp_assert(cls != nullptr, "SystemMethod::generate_inline_code() called on a SystemMethod with no containing class");
		obj = cls->getThisPtr();
		bpp_assert(obj != nullptr, "SystemMethod::generate_inline_code() called on a SystemMethod with no this pointer in its containing class");
	}

	switch (type) {
		case Type::NEW: return generateInlineNewCode(state, localize, obj);
		//case Type::DELETE: return generate_inline_delete_code(state, localize, obj);
		//case Type::COPY: return generate_inline_copy_code(state, localize, obj);
		default:
			throw bpp::ErrorHandling::InternalError("Unknown SystemMethodType in SystemMethod::generate_inline_code()");
	}
}

bpp::CodeGen::CodeSegment SystemMethod::generateInlineNewCode(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const {
	bpp_assert(state != nullptr, "SystemMethod::generate_inline_new_code() should be called with a non-null state pointer");
	bpp_assert(type == Type::NEW, "SystemMethod::generate_inline_new_code() called on a non-NEW SystemMethod");
	bpp_assert(obj != nullptr, "SystemMethod::generate_inline_new_code() called with a null object pointer");

	std::string obj_address = obj->getAddress();

	if (obj_address == "__this") obj_address = "${__this}"; // TODO(@rail5): HACK. Special-casing the @this pointer to add encasement

	const auto cls = getContainingClass().lock();
	bpp_assert(cls != nullptr, "SystemMethod::generate_inline_new_code() called on a SystemMethod with no containing class");

	bpp::CodeGen::CodeSegment result;

	std::string maybe_local = localize ? "local " : "";

	result.add_pre_code("eval \"" + maybe_local + obj_address + "____vPointer=bpp__" + cls->getName() + "____vTable\"\n");

	for (const auto& dm : cls->getAllDatamembers()) {
		if (dm->isPrimitive() || dm->isPointer()) {
			bpp::CodeGen::CodeSegment default_value_code;
			if (dm->getInitialValue().has_value()) {
				default_value_code = dm->getInitialValue().value()->generateCode(state);
			} else {
				default_value_code.add_main_code("=");
			}

			result.add_main_code(default_value_code.get_pre_code());

			if (dm->isArray()) {
				if (!dm->getInitialValue().has_value()) default_value_code.add_main_code("()");

				result.add_main_code("eval \"" + maybe_local + obj_address + dm->getAddress());
				result.add_main_code(default_value_code.get_main_code());
				result.add_main_code("\n");

				result.add_main_code(default_value_code.get_post_code());
			} else {
				result.add_main_code("local __objAssignment");
				result.add_main_code(default_value_code.get_main_code());
				result.add_main_code("\n");

				result.add_main_code("eval \"" + maybe_local + obj_address + dm->getAddress() + "=\\$__objAssignment\"\n");

				result.add_main_code(default_value_code.get_post_code());
			}

			continue;
		}

		// Non-primitive, non-pointer case
		const auto dm_cls = dm->getType().lock();
		bpp_assert(dm_cls != nullptr, "Nonprimitive data member has no type in SystemMethod::generate_inline_new_code()");

		const auto dm_new_method = dm_cls->getMethod_UNSAFE("__new");
		bpp_assert(dm_new_method != nullptr, "Class " + dm_cls->getName() + " has no __new method in SystemMethod::generate_inline_new_code()");
		bpp_assert(std::dynamic_pointer_cast<SystemMethod>(dm_new_method) != nullptr, "Class " + dm_cls->getName() + " has a non-SystemMethod __new method in SystemMethod::generate_inline_new_code()");
		const auto dm_new_sys_method = std::static_pointer_cast<SystemMethod>(dm_new_method);

		if (localize) {
			// Recursively localize 'new' for the data member
			result.absorb_all_to_main(dm_new_sys_method->generateInlineCode(state, localize, obj));
		} else {
			// If not localizing, call the __new method in a supershell, and assign its output to the datamember
			bpp::IR::Supershell sp;
			sp.inherit(shared_from_this());
			sp.add(dm_new_sys_method->getAddress());
			result.add_main_code("eval " + obj_address + dm->getAddress() + "=");
			result.egalitarian_merge(sp.generateCode(state));
		}
	}

	return result;
}


PRETTYPRINT_IMPLEMENTATION(SystemMethod, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(SystemMethod " << name << ")\n";
	return os;
})

} // namespace bpp::IR::Builtins
