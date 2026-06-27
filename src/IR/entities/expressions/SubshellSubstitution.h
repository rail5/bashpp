/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>

namespace bpp::IR {

/**
 * @brief A subshell command substitution
 * of the forms:
 * - $( command sequence ) [modern form]
 * - ` command sequence ` [deprecated form]
 * - $(< file) [builtin replacement for the `cat` command]
 *
 * For the first two forms, code generation can happen as with ordinary code entities
 *  (i.e., the generated pre- and post- code can [and should] be carried *inside* of the command substitution)
 * For the third form, code generation must be treated as a StringType code entity
 *  (i.e., the generated pre-code must *precede* the expression, and the generated post-code must *follow* the expression)
 */
class SubshellSubstitution : public StringType {
	private:
		bool is_cat_replacement = false;
	public:
		void setIsCatReplacement(bool value) { is_cat_replacement = value; }
		bool isCatReplacement() const { return is_cat_replacement; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
