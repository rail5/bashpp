#pragma once

#include "../ASTNode.h"

namespace AST {

class DynamicCast : public ASTNode {
	public:
		DynamicCast() {
			type = AST::NodeType::DynamicCast;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(DynamicCast\n"
				<< indent << "  @dynamic_cast" << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
