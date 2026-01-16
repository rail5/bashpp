/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class BashIfStatement : public ASTNode {
	public:
		BashIfStatement() {
			type = AST::NodeType::BashIfStatement;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashIfStatement if";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "fi)" << std::flush;
			return os;
		}
};

} // namespace AST
