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
	bpp_assert(state != nullptr, "State pointer is null");
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

PRETTYPRINT_IMPLEMENTATION(StringType, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
	os << indent << "(StringType\n";
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
});

} // namespace bpp::IR
