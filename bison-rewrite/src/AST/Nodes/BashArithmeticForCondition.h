#pragma once

#include "../ASTNode.h"

namespace AST {

/**
 * @class BashArithmeticForCondition
 * @brief Represents the condition part of a Bash arithmetic for loop
 * E.g., in for (( i=0; i<10; i++ )), the entire "(( i=0; i<10; i++ ))" would be represented by a BashArithmeticForCondition node.
 * Each of the three components would be represented by BashArithmeticStatement nodes as its children.
 * 
 */
class BashArithmeticForCondition : public ASTNode {
	public:
		BashArithmeticForCondition() {
			type = AST::NodeType::BashArithmeticForCondition;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashArithmeticForCondition ((\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")) )" << std::endl;
			return os;
		}
};

} // namespace AST
