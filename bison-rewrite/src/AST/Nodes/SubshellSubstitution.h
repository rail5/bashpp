#pragma once

#include "../ASTNode.h"

namespace AST {

class SubshellSubstitution : public ASTNode {
	public:
		SubshellSubstitution() {
			type = AST::NodeType::SubshellSubstitution;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(SubshellSubstitution $(\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << "))" << std::endl;
			return os;
		}
};

} // namespace AST
