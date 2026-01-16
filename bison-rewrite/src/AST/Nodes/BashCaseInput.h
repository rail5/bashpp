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
			os << indent << "(BashCaseInput";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "in)" << std::flush;
			return os;
		}
};

} // namespace AST
