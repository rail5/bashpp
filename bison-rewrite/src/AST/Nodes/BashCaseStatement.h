#pragma once

#include "../ASTNode.h"

namespace AST {

class BashCaseStatement : public ASTNode {
	public:
		BashCaseStatement() {
			type = AST::NodeType::BashCaseStatement;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashCaseStatement case\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << "esac)" << std::endl;
			return os;
		}
};

} // namespace AST
