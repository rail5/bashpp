/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/NamedEntity.h>

namespace bpp::IR {

/**
 * @brief A normal shell function
 */
class BashFunction : public CodeEntity, public NamedEntity {
	public:
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
