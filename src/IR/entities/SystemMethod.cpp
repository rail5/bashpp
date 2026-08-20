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

bpp::CodeGen::CodeSegment SystemMethod::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "SystemMethod::generate_code() should be called with a non-null state pointer");
	state->current_method = shared_from_this();

	bpp::CodeGen::CodeSegment result;

	result.add_pre_code(get_address() + "() {\n");

	for (const auto& param : get_parameters()) {
		result.absorb_all_to_pre(param->generate_code(state));
	}

	result.egalitarian_merge(generate_inline_code(state, false));

	result.add_post_code("\n}\n");


	state->current_method = nullptr;
	return result;
}

bpp::CodeGen::CodeSegment SystemMethod::generate_inline_code(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const {
	bpp_assert(state != nullptr, "SystemMethod::generate_inline_code() should be called with a non-null state pointer");

	if (!obj) {
		auto cls = get_containing_class().lock();
		bpp_assert(cls != nullptr, "SystemMethod::generate_inline_code() called on a SystemMethod with no containing class");
		obj = cls->get_this_ptr();
		bpp_assert(obj != nullptr, "SystemMethod::generate_inline_code() called on a SystemMethod with no this pointer in its containing class");
	}

	switch (type) {
		case Type::NEW: return generate_inline_new_code(state, localize, obj);
		//case Type::DELETE: return generate_inline_delete_code(state, localize, obj);
		//case Type::COPY: return generate_inline_copy_code(state, localize, obj);
		default:
			throw bpp::ErrorHandling::InternalError("Unknown SystemMethodType in SystemMethod::generate_inline_code()");
	}
}

bpp::CodeGen::CodeSegment SystemMethod::generate_inline_new_code(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const {
	bpp_assert(state != nullptr, "SystemMethod::generate_inline_new_code() should be called with a non-null state pointer");
	bpp_assert(type == Type::NEW, "SystemMethod::generate_inline_new_code() called on a non-NEW SystemMethod");
	bpp_assert(obj != nullptr, "SystemMethod::generate_inline_new_code() called with a null object pointer");

	std::string obj_address = obj->get_address();

	if (obj_address == "__this") obj_address = "${__this}"; // TODO(@rail5): HACK. Special-casing the @this pointer to add encasement

	const auto cls = get_containing_class().lock();
	bpp_assert(cls != nullptr, "SystemMethod::generate_inline_new_code() called on a SystemMethod with no containing class");

	bpp::CodeGen::CodeSegment result;

	std::string maybe_local = localize ? "local " : "";

	result.add_pre_code("eval \"" + maybe_local + obj_address + "____vPointer=bpp__" + cls->get_name() + "____vTable\"\n");

	for (const auto& dm : cls->get_datamembers()) {
		if (dm->is_primitive() || dm->is_pointer()) {
			bpp::CodeGen::CodeSegment default_value_code;
			if (dm->get_initial_value().has_value()) default_value_code = dm->get_initial_value().value()->generate_code(state);

			result.add_main_code(default_value_code.get_pre_code());

			if (dm->is_array()) {
				if (!dm->get_initial_value().has_value()) default_value_code.add_main_code("()");

				result.add_main_code("eval \"" + maybe_local + obj_address + dm->get_address());
				result.add_main_code(default_value_code.get_main_code());
				result.add_main_code("\n");

				result.add_main_code(default_value_code.get_post_code());
			} else {
				result.add_main_code("local __objAssignment");
				result.add_main_code(default_value_code.get_main_code());
				result.add_main_code("\n");

				result.add_main_code("eval \"" + maybe_local + obj_address + dm->get_address() + "=\\$__objAssignment\"\n");

				result.add_main_code(default_value_code.get_post_code());
			}

			continue;
		}

		// Non-primitive, non-pointer case
		const auto dm_cls = dm->get_type().lock();
		bpp_assert(dm_cls != nullptr, "Nonprimitive data member has no type in SystemMethod::generate_inline_new_code()");

		const auto dm_new_method = dm_cls->get_method_UNSAFE("__new");
		bpp_assert(dm_new_method != nullptr, "Class " + dm_cls->get_name() + " has no __new method in SystemMethod::generate_inline_new_code()");
		bpp_assert(std::dynamic_pointer_cast<SystemMethod>(dm_new_method) != nullptr, "Class " + dm_cls->get_name() + " has a non-SystemMethod __new method in SystemMethod::generate_inline_new_code()");
		const auto dm_new_sys_method = std::static_pointer_cast<SystemMethod>(dm_new_method);

		if (localize) {
			// Recursively localize 'new' for the data member
			result.absorb_all_to_main(dm_new_sys_method->generate_inline_code(state, localize, obj));
		} else {
			// If not localizing, call the __new method
			ObjectReference::ReferenceChain ref(obj);
			ref.append(dm);
			ref.set_method(dm_new_sys_method);

			std::shared_ptr<ObjectReference> obj_ref = std::make_shared<ObjectReference>();
			obj_ref->set_reference_chain(std::move(ref));

			result.absorb_all_to_main(obj_ref->generate_code(state));
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
