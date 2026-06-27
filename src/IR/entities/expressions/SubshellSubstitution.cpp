/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "SubshellSubstitution.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment SubshellSubstitution::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "SubshellSubstitution::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;
	result.add_main_code("$(");
	if (is_cat_replacement) {
		// egliatarian_merge places the pre-code before the main code, and the post-code after the main code
		result.egalitarian_merge(StringType::generate_code(state));
	} else {
		// otherwise, place all of it inside the main code
		result.absorb_all_to_main(StringType::generate_code(state));
	}
	result.add_main_code(")");
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
