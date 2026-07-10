/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "ValueAssignment.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment ValueAssignment::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "ValueAssignment::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;

	// FIXME(@rail5): Handle non-primitive lvalues and rvalues (this currently only handles primitive assignments, var=value)

	result.add_main_code(this->is_adding() ? "+=" : "=");
	result.egalitarian_merge(StringType::generate_code(state));

	return result;
}

PRETTYPRINT_IMPLEMENTATION(ValueAssignment, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(ValueAssignment\n";
	StringType::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
})

} // namespace bpp::IR
