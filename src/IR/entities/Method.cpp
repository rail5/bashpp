/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Method.h"

#include <IR/entities/Object.h>
#include <IR/entities/Class.h>
#include <IR/entities/MethodParameter.h>
#include <IR/entities/expressions/bpp/DynamicCast.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

void Method::ParameterList::add(const MethodParameter* parameter) {
	params[next_index++] = parameter;
}

const MethodParameter* Method::ParameterList::getByIndex(std::uint32_t index) const {
	if (!params.contains(index)) return nullptr;
	return params.at(index);
}

std::vector<const MethodParameter*> Method::ParameterList::getByName(std::string_view name) const {
	std::vector<const MethodParameter*> result;
	for (const auto& [index, param] : params) {
		if (param->viewName() == name) result.push_back(param);
	}
	return result;
}

std::optional<std::uint32_t> Method::ParameterList::getHighestIndex() const {
	if (next_index == 0) return std::nullopt;
	return next_index - 1;
}

std::expected<void, NameConflictError> Method::addParameter(std::unique_ptr<MethodParameter> owned_parameter) {
	auto existing_params = parameters.getByName(owned_parameter->viewName());
	for (const auto* existing_param : existing_params) {
		// Don't ban '$arg' and '@arg' from coexisting in the same method, since they are unambiguous
		// But if there's already '$arg', a second '$arg' is a conflict, etc.
		if (existing_param->isPointer() == owned_parameter->isPointer()) {
			return std::unexpected(NameConflictError::EXISTING_PARAMETER);
		}
	}

	if (getClass(owned_parameter->getName())) return std::unexpected(NameConflictError::EXISTING_CLASS);
	if (getObject(owned_parameter->getName())) return std::unexpected(NameConflictError::EXISTING_OBJECT);

	parameters.add(owned_parameter.get());

	// Per the spec: if a method is declared to take a pointer as a parameter,
	// then the argument passed to that parameter is implicitly dynamically cast to the expected type at the start of the method.
	if (const auto* param_type = owned_parameter->getType()) {
		auto dynamic_cast_entity = std::make_unique<DynamicCast>();
		dynamic_cast_entity->inherit(owned_parameter.get());
		dynamic_cast_entity->setTargetType(param_type->getName());
		// Tell the dynamic cast entity which positional parameter to use as its input (i.e., the argument passed to this parameter)
		dynamic_cast_entity->add("$" + std::to_string(parameters.getNextIndex()));
		// Set the initial value of this parameter to be the result of the dynamic cast
		owned_parameter->setInitialValue(std::move(dynamic_cast_entity));

		// Add to our local list of owned objects
		local_objects.add(std::move(owned_parameter));
	} else {
		// Add to our local list of owned primitive parameters
		primitive_parameters.add(std::move(owned_parameter));
	}

	return {};
}

std::expected<void, NameConflictError> Method::addParameter(const MethodParameter* unowned_parameter) {
	auto existing_params = parameters.getByName(unowned_parameter->viewName());
	for (const auto* existing_param : existing_params) {
		if (existing_param->isPointer() == unowned_parameter->isPointer()) {
			return std::unexpected(NameConflictError::EXISTING_PARAMETER);
		}
	}

	if (getClass(unowned_parameter->getName())) return std::unexpected(NameConflictError::EXISTING_CLASS);
	if (getObject(unowned_parameter->getName())) return std::unexpected(NameConflictError::EXISTING_OBJECT);

	parameters.add(unowned_parameter);
	return {};
}

std::string Method::getAddress() const {
	bpp_assert(getContainingClass(), "Method does not have a containing class");
	if (m_points_to_parent_method) {
		bpp_assert(getParentMethod() != nullptr, "Method points to parent method but has no parent method");
		return getParentMethod()->getAddress();
	}
	return "bpp__" + getContainingClass()->getName() + "__" + name;
}

bpp::CodeGen::CodeSegment Method::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	if (m_points_to_parent_method) return {}; // Skip generating code for non-overridden inherited methods
	state->current_method = this;
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code(getAddress() + "() {\n");
	code.add_pre_code("local __scopeFrames=(0)\n");

	for (const auto [index, param] : parameters.view()) {
		code.absorb_all_to_main(param->generateCode(state, index));
	}

	// We deliberately call CodeEntity::generate_code() here, rather than BashFunction::generate_code(),
	// because BashFunction::generate_code() would add a function header and footer (and that without its proper mangled name from get_address())
	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	code.absorb_all_to_main(CodeEntity::generateCode(state));

	// Destroy all local stack-like objects that were created in this method, in reverse order of creation
	code.absorb_all_to_main(destroyLocalObjects(state));

	code.add_post_code("}\n");

	state->current_method = nullptr;
	return code;
}

PRETTYPRINT_IMPLEMENTATION(Method, {
	if (m_points_to_parent_method) return os; // Skip pretty-printing non-overridden inherited methods, it would be redundant
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(Method: " << name << " [";
	switch (getScope()) {
		case VisibilityScope::INACCESSIBLE: os << "inaccessible"; break;
		case VisibilityScope::PUBLIC: os << "public"; break;
		case VisibilityScope::PRIVATE: os << "private"; break;
		case VisibilityScope::PROTECTED: os << "protected"; break;
	}
	if (m_is_virtual) os << ", virtual";
	if (getParentMethod()) os << ", inherited";
	os << "]\n";
	for (const auto [index, param] : parameters.view()) {
		param->prettyPrint(os, indentation_level + 1);
	}

	// Similar to above, we call CodeEntity::prettyPrint() here, rather than BashFunction::prettyPrint(), on purpose
	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
