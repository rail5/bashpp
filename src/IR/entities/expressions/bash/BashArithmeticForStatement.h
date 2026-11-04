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

/**
 * @brief A C-style arithmetic for loop, e.g., `for (( i=0; i<10; i++ )); do ...; done`
 */
class BashArithmeticForStatement : public CodeEntity {
	private:
		std::unique_ptr<StringType> condition;
	public:
		const StringType* getCondition() const { return condition.get(); }
		void setCondition(std::unique_ptr<StringType> cond) { condition = std::move(cond); }
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
