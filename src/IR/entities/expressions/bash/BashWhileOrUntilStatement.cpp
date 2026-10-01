/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashWhileOrUntilStatement.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashWhileOrUntilStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(condition != nullptr, "Condition is null");
	state->nested_loop_depth++;
	bpp::CodeGen::CodeSegment result;

	result.add_main_code(is_until ? "until " : "while ");
	result.egalitarian_merge(condition->generateCode(state));
	result.add_main_code("; do\n");

	result.add_main_code("__scopeFrames+=(0)\n");
	result.add_main_code("declare -a __loopFrames\n");
	result.add_main_code("__loopFrames+=(${#__scopeFrames[@]})\n"); // Used for breaking out of nested loops:
	// After entering a loop, we store an index into 'scopeFrames' at the top of the 'loopFrames' stack.
	//   This 'loopFrames' stack shows us which scopeFrames belong to loops, in order.
	// When we exit the loop, the top of the 'loopFrames' stack tells us how many scopeFrames to pop,
	//   to unwind the stack back to the state it was in before entering the loop.
	// This is so that we don't destroy local objects from outer loop(s) when breaking out of nested loops.

	result.absorb_all_to_main(CodeEntity::generateCode(state));

	result.absorb_all_to_main(destroyLocalObjects(state));

	result.add_main_code("unset __loopFrames[-1]\n");

	result.add_main_code("\ndone\n");
	state->nested_loop_depth--;
	return result;
}

PRETTYPRINT_IMPLEMENTATION(BashWhileOrUntilStatement, {
	bpp_assert(condition != nullptr, "Condition is null");
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashWhileOrUntilStatement\n" << indent;
	os << (is_until ? "until" : "while") << "\n";
	condition->prettyPrint(os, indentation_level + 1);
	os << indent << "do\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << "done)\n";
	return os;
});

} // namespace bpp::IR
