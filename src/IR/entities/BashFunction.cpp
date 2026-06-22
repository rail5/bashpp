/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashFunction.h"

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashFunction::generate_code() {
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code(name + "() {\n");
	code.egalitarian_merge(CodeEntity::generate_code());
	code.add_post_code("}\n");

	return code;
}

std::ostream& BashFunction::prettyPrint(std::ostream& os, size_t indentation_level) const {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashFunction: " << name << "\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
}

} // namespace bpp::IR
