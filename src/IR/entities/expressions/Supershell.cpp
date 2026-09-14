/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "Supershell.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment Supershell::generateCode(bpp::CodeGen::CodeGenState* state) const {
	return Supershell::wrap(state, StringType::generateCode(state));
}

bpp::CodeGen::CodeSegment Supershell::wrap(bpp::CodeGen::CodeGenState* state, std::string&& supershell_body) {
	bpp::CodeGen::CodeSegment c;
	c.add_main_code(std::move(supershell_body));
	return wrap(state, std::move(c));
}

bpp::CodeGen::CodeSegment Supershell::wrap(bpp::CodeGen::CodeGenState* state, bpp::CodeGen::CodeSegment&& supershell_body) {
	bpp_assert(state != nullptr, "State pointer is null");
	state->nested_supershell_depth++;
	bpp::CodeGen::CodeSegment result;

	const std::string supershell_function_name = "bpp____supershellF" + std::to_string(state->supershell_counter);
	const std::string supershell_output_variable = "bpp____supershellO" + std::to_string(state->supershell_counter);
	state->supershell_counter++;

	result.add_pre_code(supershell_function_name + "() {\n");
	result.absorb_all_to_pre(std::move(supershell_body));
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

	state->nested_supershell_depth--;

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
