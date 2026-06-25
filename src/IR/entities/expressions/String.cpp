/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <variant>

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment StringType::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "StringType::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;

	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			result.add_main_code(std::get<RawCode>(child));
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			const auto child_entity = std::get<std::shared_ptr<Entity>>(child);
			result.egalitarian_merge(child_entity->generate_code(state));
		}
	}

	return result;
}

bpp::CodeGen::CodeSegment String::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "String::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;
	// Surround the result of StringType::generate_code() with double quotes
	result.add_main_code("\"");
	result.egalitarian_merge(StringType::generate_code(state));
	result.add_main_code("\"");

	return result;
}

} // namespace bpp::IR
