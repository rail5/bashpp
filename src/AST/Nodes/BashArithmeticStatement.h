/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace AST {

/**
 * @class BashArithmeticStatement
 * @brief Represents a single arithmetic statement in an arithmetic for loop condition.
 * E.g., in for (( i=0; i<10; i++ )), each of "i=0", "i<10", and "i++" would be represented by a BashArithmeticStatement node.
 * 
 */
class BashArithmeticStatement : public StringType {
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::BashArithmeticStatement;
		constexpr AST::NodeType getType() const override { return static_type; }

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashArithmeticStatement";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
