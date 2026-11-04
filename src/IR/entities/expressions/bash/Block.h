/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>

namespace bpp::IR {

/**
 * @brief A curly-braced block of code, e.g., `{ ... }`, which does not otherwise belong to a larger construct (like a function, loop, or conditional).
 * Blocks are closed scopes in Bash++.
 */
class Block : public CodeEntity {
	public:
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
