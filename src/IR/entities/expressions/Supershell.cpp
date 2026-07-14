/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "Supershell.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment Supershell::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Supershell::generate_code() should be called with a non-null state pointer");
	state->supershell_stack.push({});
	bpp::CodeGen::CodeSegment result;

	const std::string supershell_function_name = "bpp____supershellF" + std::to_string(state->supershell_counter);
	const std::string supershell_output_variable = "bpp____supershellO" + std::to_string(state->supershell_counter);
	state->supershell_counter++;

	result.add_pre_code(supershell_function_name + "() {\n");
	result.absorb_all_to_pre(StringType::generate_code(state));
	result.add_pre_code("\n}\n");

	result.add_post_code("\nunset -f " + supershell_function_name + "\n");

	if (state->target_bash_version >= BashVersion{5, 3}) {
		// Bash 5.3+: use native supershell implementation
		result.add_main_code("${ " + supershell_function_name + "; }");
	} else {
		// Bash <5.3: call the supershell function
		result.add_pre_code("bpp____supershell " + supershell_output_variable + " " + supershell_function_name + "\n");
		result.add_post_code("unset " + supershell_output_variable + "\n");
		result.add_main_code("${" + supershell_output_variable + "}");
	}

	state->supershell_stack.pop();

	return result;
}

PRETTYPRINT_IMPLEMENTATION(Supershell, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(Supershell\n";
	StringType::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
