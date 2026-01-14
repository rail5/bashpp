#pragma once

#include "../ASTNode.h"
#include <optional>

namespace AST {

class DestructorDefinition : public ASTNode {
	public:
		DestructorDefinition() {
			type = AST::NodeType::DestructorDefinition;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(DestructorDefinition\n"
				<< indent << "  @destructor\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
