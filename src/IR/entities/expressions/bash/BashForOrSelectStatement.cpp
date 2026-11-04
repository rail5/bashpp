/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashForOrSelectStatement.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashForOrSelectStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(in_condition != nullptr, "For/select statement has no 'in' condition");
	bpp_assert(!variable.empty(), "For/select statement has no variable name");
	state->nested_loop_depth++;
	bpp::CodeGen::CodeSegment result;

	result.add_main_code(is_select ? "select " : "for ");
	result.add_main_code(variable + " in ");
	result.egalitarian_merge(in_condition->generateCode(state));
	result.add_main_code("; do\n");

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

PRETTYPRINT_IMPLEMENTATION(BashForOrSelectStatement, {
	bpp_assert(in_condition != nullptr, "For/select statement has no 'in' condition");
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashForOrSelectStatement\n" << indent;
	os << (is_select ? "select" : "for") << " " << variable << " in\n";
	in_condition->prettyPrint(os, indentation_level + 1);
	os << indent << "do\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << "done)\n";
	return os;
});

} // namespace bpp::IR
