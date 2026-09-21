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
	bpp_assert(state != nullptr, "State pointer is null");
	state->current_method = shared_from_this();

	bpp::CodeGen::CodeSegment result;

	result.add_pre_code(getAddress() + "() {\n");

	for (const auto& param : getParameters()) {
		result.absorb_all_to_pre(param->generateCode(state));
	}

	switch (type) {
		case Type::NEW:
			result.egalitarian_merge(generateInlineNewCode(state, false));
			result.add_main_code("\necho ${__this}\n"); // __new should echo the address of the new object, so that it can be captured by the caller
			break;
		case Type::COPY:
			result.egalitarian_merge(generateCopyCode(state));
			break;
		case Type::DELETE:
			result.egalitarian_merge(generateDeleteCode(state));
			break;
		default:
			throw bpp::ErrorHandling::InternalError("Unknown SystemMethod type");
	}

	result.add_post_code("}\n");

	state->current_method = nullptr;
	return result;
}

bpp::CodeGen::CodeSegment SystemMethod::generateInlineNewCode(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(type == Type::NEW, "System method type is not NEW");

	if (!obj) {
		auto cls = getContainingClass().lock();
		bpp_assert(cls != nullptr, "Containing class is null");
		obj = cls->getThisPtr();
		bpp_assert(obj != nullptr, "SystemMethod's containing class has no @this pointer");
	}

	std::string obj_address = obj->getAddress();

	if (obj_address == "__this") obj_address = "${__this}"; // TODO(@rail5): HACK. Special-casing the @this pointer to add encasement

	const auto cls = getContainingClass().lock();
	bpp_assert(cls != nullptr, "Containing class is null");

	bpp::CodeGen::CodeSegment result;

	if (localize) result.add_pre_code("local " + obj_address + "____vPointer\n");
	result.add_pre_code("printf -v \"" + obj_address + "____vPointer\" '%s' \"bpp__" + cls->getName() + "____vTable\"\n");

	for (const auto& dm : cls->getAllDatamembers()) {
		if (dm->isPrimitive()) {
			bpp::CodeGen::CodeSegment default_value_code;
			if (dm->getInitialValue().has_value()) {
				default_value_code = dm->getInitialValue().value()->generateCode(state);
			} else {
				default_value_code.add_main_code("=");
			}

			result.add_main_code(default_value_code.get_pre_code());

			if (localize) result.add_main_code("local " + obj_address + dm->getAddress());
			result.add_main_code("local -n __ref=" + obj_address + dm->getAddress());
			result.add_main_code("\n__ref");
			result.add_main_code(default_value_code.get_main_code());
			result.add_main_code("\n");
			result.add_main_code(default_value_code.get_post_code());

			continue;
		}

		// Non-primitive, non-pointer case
		const auto dm_cls = dm->getType().lock();
		bpp_assert(dm_cls != nullptr, "Nonprimitive data member has no type");

		const auto dm_new_method = dm_cls->getMethod_UNSAFE("__new");
		bpp_assert(dm_new_method != nullptr, "Class " + dm_cls->getName() + " has no __new method");
		bpp_assert(std::dynamic_pointer_cast<SystemMethod>(dm_new_method) != nullptr, "Class " + dm_cls->getName() + " has a non-SystemMethod __new method");
		const auto dm_new_sys_method = std::static_pointer_cast<SystemMethod>(dm_new_method);

		if (localize) {
			// Recursively localize 'new' for the data member
			result.absorb_all_to_main(dm_new_sys_method->generateInlineNewCode(state, localize, obj));
		} else {
			// If not localizing, call the __new method in a supershell, and assign its output to the datamember
			result.add_main_code("printf -v \"" + obj_address + dm->getAddress() + "\" '%s' \"");
			result.egalitarian_merge(bpp::IR::Supershell::wrap(state, dm_new_sys_method->getAddress()));
			result.add_main_code("\"\n");
		}
	}

	return result;
}

bpp::CodeGen::CodeSegment SystemMethod::generateCopyCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(type == Type::COPY, "SystemMethod is not COPY");

	const auto cls = getContainingClass().lock();
	bpp_assert(cls != nullptr, "Containing class is null");

	bpp::CodeGen::CodeSegment result;

	result.add_pre_code(R"(printf -v "${__this}____vPointer" '%s' ")" + cls->getName() + "____vTable\"\n");

	for (const auto& dm : cls->getAllDatamembers()) {
		if (dm->isPrimitive()) {
			result.add_main_code("local -n __src_ref=${__source}" + dm->getAddress() + "\n");
			result.add_main_code("local -n __dst_ref=${__this}" + dm->getAddress() + "\n");
			result.add_main_code("local __attrs=${__src_ref@a}\n"); // At runtime, get the TYPE of variable

			result.add_main_code("case \"$__attrs\" in\n"); // Runtime-determined procedure based on the type

			result.add_main_code("*A*)\n"); // Associative array case
			result.add_main_code("unset ${!__dst_ref}\ndeclare -g -A ${!__dst_ref}\n"); // Remove the existing variable, declare the new one with the "associative" attribute
			result.add_main_code("for __k in \"${!__src_ref[@]}\"; do\n");
			result.add_main_code("__dst_ref[\"$__k\"]=${__src_ref[\"$__k\"]}\n"); // Slow: copy elements one-by-one
			result.add_main_code("done\n;;\n");

			result.add_main_code("*a*)\n"); // Normal (indexed) array case
			result.add_main_code("__dst_ref=(${__src_ref[@]})\n;;\n");

			result.add_main_code("*)\n"); // Non-array case
			result.add_main_code("__dst_ref=${__src_ref}\n;;\n");

			result.add_main_code("esac\n");
		} else {
			// Recursively copy non-primitive data members
			const auto dm_cls = dm->getType().lock();
			bpp_assert(dm_cls != nullptr, "Nonprimitive data member has no type");
			const auto dm_copy_method = dm_cls->getMethod_UNSAFE("__copy");
			bpp_assert(dm_copy_method != nullptr, "Class " + dm_cls->getName() + " has no __copy method");
			result.add_main_code(dm_copy_method->getAddress() + " ${__this}" + dm->getAddress() + " ${__source}" + dm->getAddress() + "\n");
		}
	}

	return result;
}

bpp::CodeGen::CodeSegment SystemMethod::generateDeleteCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(type == Type::DELETE, "SystemMethod is not DELETE");

	const auto cls = getContainingClass().lock();
	bpp_assert(cls != nullptr, "Containing class is null");

	bpp::CodeGen::CodeSegment result;

	for (const auto& dm : cls->getAllDatamembers()) {
		if (dm->isPrimitive()) {
			result.add_main_code("unset ${__this}" + dm->getAddress() + "\n");
		} else {
			// Recursively delete non-primitive data members
			const auto dm_cls = dm->getType().lock();
			bpp_assert(dm_cls != nullptr, "Nonprimitive data member has no type");
			const auto dm_delete_method = dm_cls->getMethod_UNSAFE("__delete");
			bpp_assert(dm_delete_method != nullptr, "Class " + dm_cls->getName() + " has no __delete method");
			result.add_main_code(dm_delete_method->getAddress() + " ${__this}" + dm->getAddress() + "\n");
		}
	}

	result.add_main_code("unset ${__this}____vPointer\n");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(SystemMethod, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(SystemMethod " << name << ")\n";
	return os;
})

} // namespace bpp::IR::Builtins
