#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

class BashPipeline : public StringType {
	public:
		BashPipeline() {
			type = AST::NodeType::BashPipeline;
		}
		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashPipeline" << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
