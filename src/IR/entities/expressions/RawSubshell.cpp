/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "RawSubshell.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment RawSubshell::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	state->nested_subshell_depth++;
	bpp::CodeGen::CodeSegment result;
	result.add_main_code("(");
	// Unset the global object stack within the subshell,
	// ensuring the subshell can only destroy objects that were created within the subshell itself.
	result.add_main_code("unset __scopeFrames\n");
	result.add_main_code("unset __objectStack\n");
	result.add_main_code("unset __loopFrames\n");
	result.add_main_code("__scopeFrames=(0)\n");
	result.absorb_all_to_main(CodeEntity::generateCode(state));
	result.add_main_code("\nbpp____destroy_objectStack\n");
	result.add_main_code(")");
	state->nested_subshell_depth--;
	return result;
}

PRETTYPRINT_IMPLEMENTATION(RawSubshell, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(RawSubshell";
	os << "\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
