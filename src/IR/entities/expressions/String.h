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
 * @brief A StringType code entity is one which requires an egalitarian merge of its children when generating code, and is therefore treated as a single string.
 *
 * I.e., the pre-code and post-code of the children are merged into the pre-code and post-code of the StringType, respectively,
 * rather than being merged into the main code of the StringType.
 */
class StringType : public CodeEntity {
	public:
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

/**
 * @brief A double-quoted string which may contain interpolated expressions
 */
class String : public StringType {
	public:
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

} // namespace bpp::IR
