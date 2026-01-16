#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

/**
 * @class BashCasePatternHeader
 * @brief Represents the 'header' of a case pattern, i.e., the pattern to match against.
 * 
 */
class BashCasePatternHeader : public StringType {
	public:
		BashCasePatternHeader() {
			type = AST::NodeType::BashCasePatternHeader;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashCasePatternHeader";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "')')" << std::flush;
			return os;
		}
};

} // namespace AST
