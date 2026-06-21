/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <variant>

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment String::generate_code() {
	bpp::CodeGen::CodeSegment code_segment;
	code_segment.add_main_code("\"");
	for (auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			code_segment.add_main_code(std::move(std::get<RawCode>(child)));
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			auto child_entity = std::get<std::shared_ptr<Entity>>(child);
			code_segment.egalitarian_merge(child_entity->generate_code());
		}
	}
	code_segment.add_main_code("\"");

	return code_segment;
}

} // namespace bpp::IR
