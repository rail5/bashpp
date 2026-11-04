/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/expressions/String.h>

namespace bpp::IR {

class BashForOrSelectStatement : public CodeEntity {
	private:
		bool is_select = false;

		/// The variable name to assign each value in the "in" condition to. E.g., `for x in ...` would have `x` as the variable name.
		std::string variable;

		/// The "in" condition is the expression after the "in" keyword, evaluating to a list of values to iterate over. E.g., `for x in 1 2 3; do ...` would have `1 2 3` as the "in" condition.
		std::unique_ptr<StringType> in_condition;
	public:
		bool isSelect() const { return is_select; }
		void setIsSelect(bool select) { is_select = select; }
		const std::string& getVariable() const { return variable; }
		void setVariable(const std::string& var) { variable = var; }
		const StringType* getInCondition() const { return in_condition.get(); }
		void setInCondition(std::unique_ptr<StringType> cond) { in_condition = std::move(cond); }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
