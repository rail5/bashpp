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

class BashWhileOrUntilStatement : public CodeEntity {
	private:
		bool is_until = false;
		std::shared_ptr<StringType> condition;
	public:
		bool isUntil() const { return is_until; }
		void setIsUntil(bool until) { is_until = until; }
		std::shared_ptr<StringType> getCondition() const { return condition; }
		void setCondition(std::shared_ptr<StringType> cond) { condition = std::move(cond); }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
