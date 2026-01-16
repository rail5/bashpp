#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

class DoublequotedString : public StringType {
	public:
		DoublequotedString() {
			type = AST::NodeType::DoublequotedString;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(DoublequotedString \"";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "\")" << std::flush;
			return os;
		}
};

} // namespace AST
