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
 * @class BashArithmeticForCondition
 * @brief Represents the condition part of a Bash arithmetic for loop
 * E.g., in for (( i=0; i<10; i++ )), the entire "(( i=0; i<10; i++ ))" would be represented by a BashArithmeticForCondition node.
 * Each of the three components would be represented by BashArithmeticStatement nodes as its children.
 * 
 */
class BashArithmeticForCondition : public StringType {
	public:
		constexpr BashArithmeticForCondition() : StringType(bpp::AST::NodeType::BashArithmeticForCondition) {}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashArithmeticForCondition ((";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")) )" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
