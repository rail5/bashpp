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
 * @class BashArithmeticStatement
 * @brief Represents a single arithmetic statement in an arithmetic for loop condition.
 * E.g., in for (( i=0; i<10; i++ )), each of "i=0", "i<10", and "i++" would be represented by a BashArithmeticStatement node.
 * 
 */
class BashArithmeticStatement : public StringType {
	public:
		constexpr BashArithmeticStatement() : StringType(bpp::AST::NodeType::BashArithmeticStatement) {}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashArithmeticStatement";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
