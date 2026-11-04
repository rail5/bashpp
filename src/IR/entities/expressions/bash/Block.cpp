/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Block.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment Block::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");

	bpp::CodeGen::CodeSegment result;

	result.add_main_code("{\n");
	result.add_main_code("__scopeFrames+=(0)\n");
	result.absorb_all_to_main(CodeEntity::generateCode(state));
	result.absorb_all_to_main(destroyLocalObjects(state));
	result.add_main_code("\n}\n");
	return result;
}

PRETTYPRINT_IMPLEMENTATION(Block, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(Block\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
});

} // namespace bpp::IR
