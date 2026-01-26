/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/ASTNode.h>

namespace AST {

class BashFunction : public ASTNode {
	protected:
		AST::Token<std::string> m_NAME;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::BashFunction;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& NAME() const {
			return m_NAME;
		}
		void setName(const AST::Token<std::string>& name) {
			m_NAME = name;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
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
