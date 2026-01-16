#pragma once

#include "../ASTNode.h"

namespace AST {

class BashCaseInput : public ASTNode {
	public:
		BashCaseInput() {
			type = AST::NodeType::BashCaseInput;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashCaseInput\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << "in)" << std::endl;
			return os;
		}
};

} // namespace AST
