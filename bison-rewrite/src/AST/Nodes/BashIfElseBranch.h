#pragma once

#include "../ASTNode.h"

namespace AST {

class BashIfElseBranch : public ASTNode {
	public:
		BashIfElseBranch() {
			type = AST::NodeType::BashIfElseBranch;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashIfElseBranch";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
