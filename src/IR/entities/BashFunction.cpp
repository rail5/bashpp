/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashFunction.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashFunction::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "BashFunction::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	state->nested_bash_function_depth++;

	code.add_pre_code(name + "() {\n");
	code.egalitarian_merge(CodeEntity::generateCode(state));
	code.add_post_code("}\n");

	state->nested_bash_function_depth--;

	return code;
}

PRETTYPRINT_IMPLEMENTATION(BashFunction, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashFunction: " << name << "\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
