/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class PrimitiveAssignment : public ASTNode {
	protected:
		AST::Token<std::string> m_IDENTIFIER;
		bool m_local = false;
	public:
		constexpr PrimitiveAssignment() : ASTNode(bpp::AST::NodeType::PrimitiveAssignment) {}

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

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(PrimitiveAssignment\n"
				<< indent << "  " << (m_local ? "local " : "") << m_IDENTIFIER << "=";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
