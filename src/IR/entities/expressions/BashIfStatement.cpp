/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashIfStatement.h"

#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment BashIfStatement::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment result;

	for (const auto& branch : branches) {
		result.egalitarian_merge(branch->generateCode(state));
	}
	result.add_main_code("fi\n");
	return result;
}

PRETTYPRINT_IMPLEMENTATION(BashIfStatement, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashIfStatement\n";
	for (const auto& branch : branches) {
		branch->prettyPrint(os, indentation_level + 1);
	}
	os << indent << ")\n";
	return os;
});

bpp::CodeGen::CodeSegment BashIfBranch::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");

	bpp::CodeGen::CodeSegment result;

	if (root_branch) {
		result.add_main_code("if ");
	} else {
		if (condition.has_value()) {
			result.add_main_code("elif ");
		} else {
			result.add_main_code("else");
		}
	}

	if (condition.has_value()) {
		result.egalitarian_merge(condition.value()->generateCode(state));
		result.add_main_code("; then");
	}

	result.add_main_code("\n");

	result.absorb_all_to_main(CodeEntity::generateCode(state));
	return result;
}

PRETTYPRINT_IMPLEMENTATION(BashIfBranch, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashIfBranch";
	if (root_branch) {
		os << " [root branch]";
	} else {
		if (condition.has_value()) {
			os << " [elif branch]";
		} else {
			os << " [else branch]";
		}
	}
	os << "\n";
	if (condition.has_value()) {
		condition.value()->prettyPrint(os, indentation_level + 1);
	}
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
});

PRETTYPRINT_IMPLEMENTATION(BashIfCondition, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(BashIfCondition\n";
	StringType::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
});

} // namespace bpp::IR
