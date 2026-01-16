#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

class HereString : public StringType {
	public:
		HereString() {
			type = AST::NodeType::HereString;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(HereString <<<";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
