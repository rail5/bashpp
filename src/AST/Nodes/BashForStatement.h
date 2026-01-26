/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/ASTNode.h>

namespace AST {

class BashForStatement : public ASTNode {
	protected:
		AST::Token<std::string> m_VARIABLE;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::BashForStatement;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& VARIABLE() const {
			return m_VARIABLE;
		}

		void setVariable(const AST::Token<std::string>& variable) {
			m_VARIABLE = variable;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashForStatement\n"
				<< indent << "  for " << m_VARIABLE;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
