#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

/**
 * @class BashInCondition
 * @brief Represents a Bash 'in' condition used in for loops and select statements.
 * 
 */
class BashInCondition : public StringType {
	public:
		BashInCondition() {
			type = AST::NodeType::BashInCondition;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashInCondition in";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
