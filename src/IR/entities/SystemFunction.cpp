/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <IR/bpp.h>
#include "SystemFunction.h"

#include <error/InternalError.h>

namespace bpp::IR::Builtins {

bpp::CodeGen::CodeSegment SystemFunction::generate_code(bpp::CodeGen::CodeGenState* /*state*/) const {
	if (!this->is_referenced()) return {};
	bpp::CodeGen::CodeSegment result;
	result.add_main_code(std::string(this->m_contents));
	return result;
}

PRETTYPRINT_IMPLEMENTATION(SystemFunction, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(SystemFunction)\n";
	return os;
})

} // namespace bpp::IR::Builtins
