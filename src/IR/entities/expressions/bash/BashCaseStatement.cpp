/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashCaseStatement.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashCasePattern::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(pattern_header != nullptr, "Pattern header is null");

	bpp::CodeGen::CodeSegment result;
	result.egalitarian_merge(pattern_header->generateCode(state));
	result.add_main_code(")\n");
	result.absorb_all_to_main(CodeEntity::generateCode(state));
	return result;
}

PRETTYPRINT_IMPLEMENTATION(BashCasePattern, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashCasePattern\n";
	if (pattern_header) pattern_header->prettyPrint(os, indentation_level + 1);
	StringType::prettyPrint(os, indentation_level + 2);
	os << indent << ")\n";
	return os;
});

bpp::CodeGen::CodeSegment BashCaseStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp_assert(case_input != nullptr, "Case input is null");

	bpp::CodeGen::CodeSegment result;
	result.add_main_code("case ");
	result.egalitarian_merge(case_input->generateCode(state));
	result.add_main_code(" in\n");
	result.egalitarian_merge(StringType::generateCode(state));
	result.add_main_code("esac\n");
	return result;
}

PRETTYPRINT_IMPLEMENTATION(BashCaseStatement, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashCaseStatement\n";
	if (case_input) case_input->prettyPrint(os, indentation_level + 1);
	StringType::prettyPrint(os, indentation_level + 2);
	os << indent << ")\n";
	return os;
});

} // namespace bpp::IR
