#pragma once

#include "../ASTNode.h"
#include <optional>

namespace AST {

class ConstructorDefinition : public ASTNode {
	public:
		ConstructorDefinition() {
			type = AST::NodeType::ConstructorDefinition;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(ConstructorDefinition\n"
				<< indent << "  @constructor\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
