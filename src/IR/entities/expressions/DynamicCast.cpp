/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DynamicCast.h"

#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment DynamicCast::generate_code() {
	bpp::CodeGen::CodeSegment result;

	const auto& inner_code = String::generate_code();

	result.add_pre_code(inner_code.get_pre_code());
	result.add_post_code(inner_code.get_post_code());

	const std::string reference_value = [&inner_code]() {
		std::string result;
		for (const auto& part : inner_code.get_main_code()) result += part;
		return result;
	}();

	// The 'main code' of the inner expression is the reference value, i.e., the address of the object to be casted.
	auto program = containing_program.lock();
	bpp_assert(program != nullptr, "DynamicCast does not have a containing program");

	const std::string resulting_temporary_variable = "__dynamicCast" + std::to_string(program->codegen_state.dynamic_cast_counter++);

	result.add_pre_code("bpp__dynamic_cast \"" + target_type->get_name() + "\""
		+ " \"" + resulting_temporary_variable + "\""
		+ " " + reference_value + "\n");
	
	result.add_post_code("unset " + resulting_temporary_variable + "\n");

	result.add_main_code("${" + resulting_temporary_variable + "}");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(DynamicCast, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(DynamicCast to " << target_type->get_name() << "\n";
	String::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
