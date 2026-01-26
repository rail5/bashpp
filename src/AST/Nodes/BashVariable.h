/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/ASTNode.h>

namespace AST {

class BashVariable : public ASTNode {
	protected:
		AST::Token<std::string> m_TEXT;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::BashVariable;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& TEXT() const {
			return m_TEXT;
		}

		void setText(const AST::Token<std::string>& text) {
			m_TEXT = text;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashVariable\n"
				<< indent << "  ${" << m_TEXT;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "})" << std::flush;
			return os;
		}
};

} // namespace AST
