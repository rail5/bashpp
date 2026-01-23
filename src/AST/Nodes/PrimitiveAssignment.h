/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class PrimitiveAssignment : public ASTNode {
	protected:
		AST::Token<std::string> m_IDENTIFIER;
		bool m_local = false;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::PrimitiveAssignment;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& IDENTIFIER() const {
			return m_IDENTIFIER;
		}
		void setIdentifier(const AST::Token<std::string>& identifier) {
			m_IDENTIFIER = identifier;
		}

		bool isLocal() const {
			return m_local;
		}
		void setLocal(bool local) {
			m_local = local;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(PrimitiveAssignment\n"
				<< indent << "  " << (m_local ? "local " : "") << m_IDENTIFIER << "=";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
