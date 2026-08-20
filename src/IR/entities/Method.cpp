/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Method.h"

#include <IR/entities/Object.h>
#include <IR/entities/expressions/DynamicCast.h>

#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bool Method::add_parameter(std::shared_ptr<MethodParameter> parameter) {
	for (const auto& p : parameters) {
		if (p->get_name() == parameter->get_name()) return false; // Parameter with this name already exists
	}

	// The @this pointer will always be given an index of 1
	// But, after @this, we shift the positional arguments of the method,
	// so the next parameter is *also* given an index of 1, then 2, then 3, etc.
	parameter->set_index(std::max(static_cast<std::uint32_t>(parameters.size()), 1u));

	// Per the spec: if a method is declared to take a pointer as a parameter,
	// then the argument passed to that parameter is implicitly dynamically cast to the expected type at the start of the method.
	if (auto param_type = parameter->get_type().lock()) {
		// Verify that this parameter's name doesn't conflict with any known classes or objects
		if (get_object(parameter->get_name()) || get_class(parameter->get_name())) return false;

		auto dynamic_cast_entity = std::make_shared<DynamicCast>();
		dynamic_cast_entity->inherit(parameter);
		dynamic_cast_entity->set_target_type(param_type->get_name());
		// Tell the dynamic cast entity which positional parameter to use as its input (i.e., the argument passed to this parameter)
		dynamic_cast_entity->add("$" + std::to_string(parameter->get_index()));
		// Set the initial value of this parameter to be the result of the dynamic cast
		parameter->set_initial_value(dynamic_cast_entity);

		// Mark the dynamic_cast builtin as referenced by this parameter
		auto containing_program = get_containing_program().lock();
		bpp_assert(containing_program != nullptr, "MethodParameter does not have a containing program");
		auto dynamic_cast_builtin = containing_program->get_dynamic_cast_function();
		bpp_assert(dynamic_cast_builtin != nullptr, "Containing program does not have a dynamic_cast builtin");
		dynamic_cast_builtin->mark_referenced_by(parameter);

		// Add to our local list of owned objects, so that it can be found by name later
		local_objects.add(parameter);
	}

	parameters.push_back(parameter);
	return true;
}

void Method::add_reference_position(const SymbolPosition& pos) {
	Entity::add_reference_position(pos);
	if (auto parent = parent_method.lock()) {
		parent->add_reference_position(pos);
	}
}

std::string Method::get_address() const {
	bpp_assert(!containing_class.expired(), "Method does not have a containing class");
	return "bpp__" + containing_class.lock()->get_name() + "__" + name;
}

bpp::CodeGen::CodeSegment Method::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Method::generate_code() should be called with a non-null state pointer");
	state->current_method = shared_from_this();
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code(get_address() + "() {\n");

	for (const auto& param : parameters) {
		code.absorb_all_to_main(param->generate_code(state));
	}

	// We deliberately call CodeEntity::generate_code() here, rather than BashFunction::generate_code(),
	// because BashFunction::generate_code() would add a function header and footer (and that without its proper mangled name from get_address())
	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	code.absorb_all_to_main(CodeEntity::generate_code(state));

	code.add_post_code("}\n");

	state->current_method = nullptr;
	return code;
}

PRETTYPRINT_IMPLEMENTATION(Method, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(Method: " << name << " [";
	switch (scope) {
		case VisibilityScope::INACCESSIBLE: os << "inaccessible"; break;
		case VisibilityScope::PUBLIC: os << "public"; break;
		case VisibilityScope::PRIVATE: os << "private"; break;
		case VisibilityScope::PROTECTED: os << "protected"; break;
	}
	if (m_is_virtual) os << ", virtual";
	if (m_is_inherited) os << ", inherited";
	os << "]\n";
	for (const auto& param : parameters) {
		param->prettyPrint(os, indentation_level + 1);
	}

	// Similar to above, we call CodeEntity::prettyPrint() here, rather than BashFunction::prettyPrint(), on purpose
	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
