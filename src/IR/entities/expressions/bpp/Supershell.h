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
 * @brief A supershell command substitution
 * of the form `@( command sequence )`
 */
class Supershell : public StringType {
	public:
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		static bpp::CodeGen::CodeSegment wrap(bpp::CodeGen::CodeGenState* state, bpp::CodeGen::CodeSegment&& supershell_body);
		static bpp::CodeGen::CodeSegment wrap(bpp::CodeGen::CodeGenState* state, std::string&& supershell_body);
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
