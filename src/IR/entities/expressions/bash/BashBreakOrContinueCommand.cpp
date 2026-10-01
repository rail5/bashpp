/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashBreakOrContinueCommand.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashBreakOrContinueCommand::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State is null");
	bpp::CodeGen::CodeSegment result;

	// Firstly: if this can't possibly be an exit point, just emit the command as-is, without any special handling
	if (!is_exit_path || state->nested_loop_depth == 0) {
		result.add_main_code(is_break ? "break " : "continue ");
		result.egalitarian_merge(StringType::generateCode(state));
		return result;
	}

	// Otherwise, we need to do a few things:
	// 1. Fetch the result of the argument expression FIRST (how many loops are we breaking/continuing out of?)
	//    or default to 1 if no argument is provided
	// 2. Constrain the argument to the number of enclosing loops (state->nested_loop_depth)
	//    Bash manual says: if the argument is greater than the number of enclosing loops, it's capped at the number of enclosing loops
	// 3. Unwind the object stack to the appropriate depth (destroying local objects as we go)
	// 4. Actually break/continue out of the appropriate number of loops

	// 1. Fetch the argument expression (or default to 1 if no argument)
	bpp::CodeGen::CodeSegment arg_code;
	if (state->should_declare_local()) arg_code.add_main_code("local ");
	arg_code.add_main_code("__loopDepth=");
	auto res = StringType::generateCode(state);
	if (res.get_main_code().empty()) arg_code.add_main_code("1");
	arg_code.egalitarian_merge(std::move(res));
	arg_code.add_main_code("\n");

	result.absorb_all_to_pre(std::move(arg_code));

	// 2. Constrain the argument to the number of enclosing loops
	result.add_pre_code("__loopDepth=$((__loopDepth < " + std::to_string(state->nested_loop_depth) + " ? __loopDepth : " + std::to_string(state->nested_loop_depth) + "))\n");
	if (state->should_declare_local()) result.add_pre_code("local ");
	result.add_pre_code("__bcArg=${__loopDepth}\n");
	
	// 3. Unwind the object stack to the appropriate depth
	result.add_pre_code("while [[ ${#__loopFrames[@]} -gt 0 ]] && [[ ${__loopDepth} -gt 0 ]]; do\n");
	result.add_pre_code("while [[ ${#__scopeFrames[@]} -ge ${__loopFrames[-1]} ]]; do\n");
	result.add_pre_code("bpp____destroy_objectStack ${__scopeFrames[-1]}\n");
	result.add_pre_code("unset __scopeFrames[-1]\n");
	result.add_pre_code("done\n");
	result.add_pre_code("unset __loopFrames[-1]\n");
	result.add_pre_code("__loopDepth=$((__loopDepth - 1))\n");
	result.add_pre_code("done\n");

	// 4. Actually break/continue out of the appropriate number of loops
	result.add_main_code(is_break ? "break ${__bcArg}\n" : "continue ${__bcArg}\n");

	return result;
}

} // namespace bpp::IR
