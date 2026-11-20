/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "ValueAssignment.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment ValueAssignment::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(rvalue_reference == nullptr, "ValueAssignment::generateCode() called on a nonprimitive assignment, which should have been handled by ObjectAssignment");
	bpp::CodeGen::CodeSegment result;

	result.add_main_code(this->isAdding() ? "+=" : "=");
	result.egalitarian_merge(StringType::generateCode(state));

	return result;
}

PRETTYPRINT_IMPLEMENTATION(ValueAssignment, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ValueAssignment\n";
	if (rvalue_reference) {
		rvalue_reference->prettyPrint(os, indentation_level + 1);
	} else {
		StringType::prettyPrint(os, indentation_level + 1);
	}
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
