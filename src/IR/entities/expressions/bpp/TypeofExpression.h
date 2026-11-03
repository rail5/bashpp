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
 * @brief A `@typeof [x]` expression, where `[x]` is any rvalue expression at all.
 * `[x]` is evaluated, and the result is interpreted as a pointer to an object,
 * the type of which is substituted (read: expanded) in place of the `@typeof` expression.
 */
class TypeofExpression : public StringType {
	public:
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
