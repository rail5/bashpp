/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "SubshellSubstitution.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment SubshellSubstitution::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	state->nested_subshell_depth++;
	bpp::CodeGen::CodeSegment result;
	result.add_main_code("$(");
	if (is_cat_replacement) {
		// egliatarian_merge places the pre-code before the main code, and the post-code after the main code
		result.egalitarian_merge(StringType::generateCode(state));
		// FIXME(@rail5): ... Do we need to worry about destroying local objects in this case? Review.
	} else {
		// otherwise, place all of it inside the main code

		// Unset the global object stack within the subshell,
		// ensuring the subshell can only destroy objects that were created within the subshell itself.
		result.add_main_code("unset __scopeFrames\n");
		result.add_main_code("unset __objectStack\n");
		result.add_main_code("unset __loopFrames\n");
		result.add_main_code("__scopeFrames=(0)");
		result.absorb_all_to_main(StringType::generateCode(state));
		result.add_main_code("\nbpp____destroy_objectStack\n");
	}
	result.add_main_code(")");
	state->nested_subshell_depth--;
	return result;
}

PRETTYPRINT_IMPLEMENTATION(SubshellSubstitution, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(SubshellSubstitution";
	if (is_cat_replacement) os << " [cat replacement]";
	os << "\n";
	StringType::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
