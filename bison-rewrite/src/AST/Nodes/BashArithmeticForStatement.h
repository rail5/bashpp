#pragma once

#include "../ASTNode.h"

namespace AST {

/**
 * @class BashArithmeticForStatement
 * @brief Represents a Bash arithmetic for loop statement.
 * E.g., for (( i=0; i<10; i++ )); do ...; done
 * 
 */
class BashArithmeticForStatement : public ASTNode {
	public:
		BashArithmeticForStatement() {
			type = AST::NodeType::BashArithmeticForStatement;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashArithmeticForStatement for";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
