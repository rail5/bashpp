/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashPipeline.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashPipeline::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment result;

	auto stringTypeCodegen = StringType::generateCode(state);
	const bool has_pre_code = !stringTypeCodegen.get_pre_code().empty();
	const bool has_post_code = !stringTypeCodegen.get_post_code().empty();
	if (has_pre_code || has_post_code) {
		/*
		 * If the pipeline has either pre- or post-code (code which executes before or after the main code),
		 * Then we need to wrap the pipeline in a curly-brace block so that the shell recognizes it as a single command.
		 *
		 * E.g.:
		 *
		 *   Source: command1 && echo @obj.member | @object.method && command3
		 *
		 *   Becomes: command1 && [PRE-CODE] ; echo [MAIN-CODE1] | [MAIN-CODE2] ; [POST-CODE] && command3
		 *
		 * Because of the fact that the pre- and post-code (for the inner pipeline) are in fact separate commands,
		 * This spoils the command sequence joined with logical connectives (the && operators in the above case)
		 *
		 * However, when we wrap the pipeline in a curly-brace block:
		 *
		 *  command1 && { [PRE-CODE] ; echo [MAIN-CODE1] | [MAIN-CODE2] ; [POST-CODE] ; } && command3
		 *
		 * The curly-brace block is recognized as a single element in the command sequence, and the logical connectives work as expected.
		 */
		result.add_pre_code("{\n");
		result.egalitarian_merge(std::move(stringTypeCodegen));

		if (has_post_code) {
			/*
			 * Specifically, if the pipeline has post-code, we need to repeat the exit status of the main code *after* the post-code executes,
			 * so that the exit status of the pipeline remains the exit status of the main code, rather than the exit status of the post-code.
			 *
			 * E.g., if the user wants to run `if @obj.method; then ...`,
			 * then they want to check whether @obj.method succeeded, *not* whether whatever post-code (necessary to clean up after @obj.method) succeeded.
			 */
			state->requires_repeat_function = true;
			result.add_main_code("\n");
			if (state->should_declare_local()) result.add_main_code("local ");
			result.add_main_code("__ret=$?\n"); // Store the exit status of the main code in a temporary variable
			result.add_post_code("\nbpp____repeat $__ret\n"); // Repeat it
		}

		result.add_post_code("\n}");
	} else {
		// If the pipeline has no pre- or post-code, we don't need to do anything special.
		result = std::move(stringTypeCodegen);
	}

	switch (getExitPointType()) {
		case ExitPointType::NO_EXIT: break; // This pipeline does not cause an exit, no special procedure needed
		case ExitPointType::FUNCTION_EXIT:
			if (!state->should_declare_local()) break; // If we're not inside a generated function, 'return' should not destroy any objects
			result.add_pre_code("\nwhile [[ ${#__scopeFrames[@]} -gt 0 ]]; do\n");
			result.add_pre_code("bpp____destroy_objectStack ${__scopeFrames[-1]}\n");
			result.add_pre_code("unset __scopeFrames[-1]\n");
			result.add_pre_code("done\n");
			break;
		case ExitPointType::PROGRAM_EXIT:
			// Omitting the argument means "destroy everything"
			result.add_pre_code("\nbpp____destroy_objectStack\n");
			break;
	}

	return result;
}

} // namespace bpp::IR
