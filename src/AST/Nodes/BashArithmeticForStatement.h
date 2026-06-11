/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

/**
 * @class BashArithmeticForStatement
 * @brief Represents a Bash arithmetic for loop statement.
 * E.g., for (( i=0; i<10; i++ )); do ...; done
 * 
 */
class BashArithmeticForStatement : public ASTNode {
	public:
		constexpr BashArithmeticForStatement() : ASTNode(bpp::AST::NodeType::BashArithmeticForStatement) {}

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashArithmeticForStatement for";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace bpp::AST
