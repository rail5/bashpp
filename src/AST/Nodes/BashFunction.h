/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class BashFunction : public ASTNode {
	protected:
		AST::Token<std::string> m_NAME;
	public:
		BashFunction() {
			type = AST::NodeType::BashFunction;
		}

		const AST::Token<std::string>& NAME() const {
			return m_NAME;
		}
		void setName(const AST::Token<std::string>& name) {
			m_NAME = name;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashFunction function " << m_NAME.getValue() << "()";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
