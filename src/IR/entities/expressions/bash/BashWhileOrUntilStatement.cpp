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

	result.absorb_all_to_main(createScopeFrame(state, false, true));
	result.absorb_all_to_main(markLoopFrame(state)); // Mark this scope frame as belonging to a loop

	result.absorb_all_to_main(CodeEntity::generateCode(state));

	result.absorb_all_to_main(destroyScopeFrame(state));
	result.absorb_all_to_main(unmarkLoopFrame(state));

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
