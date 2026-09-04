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

bpp::CodeGen::CodeSegment StringType::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "StringType::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;

	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			result.copy_to_main_code(std::get<RawCode>(child));
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			const auto child_entity = std::get<std::shared_ptr<Entity>>(child);
			result.egalitarian_merge(child_entity->generateCode(state));
		}
	}

	return result;
}

bpp::CodeGen::CodeSegment String::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "String::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment result;
	// Surround the result of StringType::generate_code() with double quotes
	result.add_main_code("\"");
	result.egalitarian_merge(StringType::generateCode(state));
	result.add_main_code("\"");

	return result;
}

PRETTYPRINT_IMPLEMENTATION(String, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "\"";

	bool last_printed_was_entity = false;
	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			if (last_printed_was_entity) os << indent;
			prettyprint_raw_code(os, std::get<RawCode>(child));
			last_printed_was_entity = false;
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			os << "\n";
			std::get<std::shared_ptr<Entity>>(child)->prettyPrint(os, indentation_level);
			last_printed_was_entity = true;
		}
	}
	if (last_printed_was_entity) os << indent;
	os << "\"\n";
	return os;
})

} // namespace bpp::IR
