/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "RawSubshell.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment RawSubshell::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "RawSubshell::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;
	result.add_main_code("(");
	result.absorb_all_to_main(CodeEntity::generate_code(state));
	result.add_main_code(")");
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
