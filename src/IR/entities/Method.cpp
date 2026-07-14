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
		// In this case, this is handled by the base Object's generate_code() method,
		// assuming that the field `initial_value` has been properly set to point to the dynamic cast code entity.
		code.egalitarian_merge(Object::generate_code(state));
	}
	return code;
}

ThisPtr::ThisPtr(std::shared_ptr<Method> containing_method) : MethodParameter(containing_method) {
	set_name("this");
	set_type(containing_method->get_containing_class());
	set_is_pointer(true);
	inherit(containing_method);
}

bpp::CodeGen::CodeSegment ThisPtr::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "ThisPtr::generate_code() should be called with a non-null state pointer");
	bpp_assert(has_initial_value(), "ThisPtr has no initial value set");
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
	+ type.lock()->get_name() + "." + containing_method.lock()->get_name()
		+ " on null object\"\n"
		"\treturn 1\n"
		"fi\n"
	);

	return code;
}

bool Method::add_parameter(std::shared_ptr<MethodParameter> parameter) {
	for (const auto& p : parameters) {
		if (p->get_name() == parameter->get_name()) return false; // Parameter with this name already exists
	}

	parameter->set_index(static_cast<std::uint32_t>(parameters.size() + 1));

	// Per the spec: if a method is declared to take a pointer as a parameter,
	// then the argument passed to that parameter is implicitly dynamically cast to the expected type at the start of the method.
	if (auto param_type = parameter->get_type().lock()) {
		auto dynamic_cast_entity = std::make_shared<DynamicCast>();
		dynamic_cast_entity->inherit(parameter);
		dynamic_cast_entity->set_target_type(param_type->get_name());
		// Tell the dynamic cast entity which positional parameter to use as its input (i.e., the argument passed to this parameter)
		dynamic_cast_entity->add("$" + std::to_string(parameter->get_index()));
		// Set the initial value of this parameter to be the result of the dynamic cast
		parameter->set_initial_value(dynamic_cast_entity);
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

std::string Method::get_mangled_name() const {
	return "bpp__" + containing_class.lock()->get_name() + "__" + name;
}

bpp::CodeGen::CodeSegment Method::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Method::generate_code() should be called with a non-null state pointer");
	state->in_method = true;
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code(get_mangled_name() + "() {\n");

	for (const auto& param : parameters) {
		code.absorb_all_to_main(param->generate_code(state));
	}

	code.absorb_all_to_main(CodeEntity::generate_code(state));

	code.add_post_code("}\n");

	state->in_method = false;
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
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
