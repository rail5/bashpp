/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class BashSelectStatement : public ASTNode {
	protected:
		AST::Token<std::string> m_VARIABLE;
	public:
		constexpr BashSelectStatement() : ASTNode(bpp::AST::NodeType::BashSelectStatement) {}

		const AST::Token<std::string>& VARIABLE() const {
			return m_VARIABLE;
		}

		void setVariable(const AST::Token<std::string>& variable) {
			m_VARIABLE = variable;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashSelectStatement\n"
				<< indent << "  select " << m_VARIABLE;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
