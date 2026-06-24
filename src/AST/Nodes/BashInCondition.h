/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace bpp::AST {

/**
 * @class BashInCondition
 * @brief Represents a Bash 'in' condition used in for loops and select statements.
 * 
 */
class BashInCondition : public StringType {
	public:
		constexpr BashInCondition() : StringType(bpp::AST::NodeType::BashInCondition) {}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashInCondition in";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
