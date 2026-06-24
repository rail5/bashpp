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

bpp::CodeGen::CodeSegment String::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "String::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code_segment;
	code_segment.add_main_code("\"");
	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			code_segment.add_main_code(std::get<RawCode>(child));
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			auto child_entity = std::get<std::shared_ptr<Entity>>(child);
			code_segment.egalitarian_merge(child_entity->generate_code(state));
		}
	}
	code_segment.add_main_code("\"");

	return code_segment;
}

} // namespace bpp::IR
