/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashArithmeticForStatement.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashArithmeticForStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(condition != nullptr, "Arithmetic 'for' has no condition");
	state->nested_loop_depth++;
	bpp::CodeGen::CodeSegment result;

	result.add_main_code("for ((");
	result.egalitarian_merge(condition->generateCode(state));
	result.add_main_code(")); do\n");

	result.add_main_code("__scopeFrames+=(0)\n");
	result.add_main_code("declare -a __loopFrames\n");
	result.add_main_code("__loopFrames+=(${#__scopeFrames[@]})\n");

	result.absorb_all_to_main(CodeEntity::generateCode(state));
	result.absorb_all_to_main(destroyLocalObjects(state));

	result.add_main_code("unset __loopFrames[-1]\n");

	result.add_main_code("\ndone\n");
	state->nested_loop_depth--;
	return result;
}

PRETTYPRINT_IMPLEMENTATION(BashArithmeticForStatement, {
	bpp_assert(condition != nullptr, "Arithmetic 'for' has no condition");
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashArithmeticForStatement\n" << indent;
	os << "for ((";
	condition->prettyPrint(os, indentation_level + 1);
	os << "))\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << "done)\n";
	return os;
});

} // namespace bpp::IR
