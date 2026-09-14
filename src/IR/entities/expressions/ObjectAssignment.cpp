/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ObjectAssignment.h"

namespace bpp::IR {

bpp::CodeGen::CodeSegment ObjectAssignment::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(lhs != nullptr, "LHS is null");
	bpp_assert(rhs != nullptr, "RHS is null");

	bpp::CodeGen::CodeSegment result;

	if (state->should_declare_local()) result.add_main_code("local ");
	result.add_main_code("__assignment");
	result.egalitarian_merge(rhs->generateCode(state));
	result.add_main_code("\n");

	if (!state->should_declare_local()) result.add_post_code("\nunset __assignment\n");

	result.add_main_code("eval ");
	result.egalitarian_merge(lhs->generateCode(state));
	result.add_main_code("=\\$__assignment\n");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(ObjectAssignment, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ObjectAssignment\n";
	lhs->prettyPrint(os, indentation_level + 1);
	rhs->prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
