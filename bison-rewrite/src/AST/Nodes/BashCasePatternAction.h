#pragma once

#include "../ASTNode.h"

namespace AST {

/**
 * @class BashCasePatternAction
 * @brief Represents the action to be taken for a specific case pattern in a Bash case statement.
 * 
 */
class BashCasePatternAction : public ASTNode {
	public:
		BashCasePatternAction() {
			type = AST::NodeType::BashCasePatternAction;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashCasePatternAction\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
