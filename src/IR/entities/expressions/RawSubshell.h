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
 * @brief A "raw" subshell (not command substitution)
 * Of the form `( command sequence )`
 * In Bash, this spawns a new shell to execute the command sequence,
 * without expanding the output of the sequence in place of the subshell.
 */
class RawSubshell : public CodeEntity {
	public:
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
