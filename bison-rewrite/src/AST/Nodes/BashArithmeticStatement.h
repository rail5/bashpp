#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

/**
 * @class BashArithmeticStatement
 * @brief Represents a single arithmetic statement in an arithmetic for loop condition.
 * E.g., in for (( i=0; i<10; i++ )), each of "i=0", "i<10", and "i++" would be represented by a BashArithmeticStatement node.
 * 
 */
class BashArithmeticStatement : public StringType {
	public:
		BashArithmeticStatement() {
			type = AST::NodeType::BashArithmeticStatement;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashArithmeticStatement\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
