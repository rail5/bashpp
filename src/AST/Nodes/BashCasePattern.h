/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class BashCasePattern : public ASTNode {
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::BashCasePattern;
		constexpr AST::NodeType getType() const override { return static_type; }

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashCasePattern";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ";;)" << std::flush;
			return os;
		}
};

} // namespace AST
