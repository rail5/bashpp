/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "TypeofExpression.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment TypeofExpression::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	state->requires_typeof_function = true;

	bpp::CodeGen::CodeSegment result;
	const std::string typeof_output_variable = "__typeofO" + std::to_string(state->typeof_counter);
	state->typeof_counter++;

	if (state->should_declare_local()) result.add_pre_code("local " + typeof_output_variable + "\n");

	result.add_main_code("bpp____typeof ");
	result.egalitarian_merge(StringType::generateCode(state)); // evaluate the expression inside the @typeof
	result.add_main_code(" " + typeof_output_variable + "\n");

	result.collapse_to_pre_code(); // all that fetching is pre-code activity

	result.add_main_code("${" + typeof_output_variable + "}"); // substitute the result of the @typeof expression
	result.add_post_code("unset " + typeof_output_variable + "\n"); // clean up the temporary variable which held the result of the @typeof expression
	return result;
}

PRETTYPRINT_IMPLEMENTATION(TypeofExpression, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(@typeof\n";
	StringType::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
