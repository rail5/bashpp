#pragma once

#include "../ASTNode.h"

namespace AST {

class BashIfRootBranch : public ASTNode {
	public:
		BashIfRootBranch() {
			type = AST::NodeType::BashIfRootBranch;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashIfRootBranch\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
