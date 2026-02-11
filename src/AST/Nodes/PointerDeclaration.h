/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <AST/ASTNode.h>

namespace AST {

class PointerDeclaration : public ASTNode {
	protected:
		AST::Token<std::string> m_TYPE;
		AST::Token<std::string> m_IDENTIFIER;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::PointerDeclaration;
		constexpr AST::NodeType getType() const override { return static_type; }

		void setType(const AST::Token<std::string>& type) {
			m_TYPE = type;
		}

		const AST::Token<std::string>& TYPE() const {
			return m_TYPE;
		}

		void setIdentifier(const AST::Token<std::string>& identifier) {
			m_IDENTIFIER = identifier;
		}

		const AST::Token<std::string>& IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(PointerDeclaration\n"
				<< indent << "  @" << m_TYPE << "* " << m_IDENTIFIER;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
