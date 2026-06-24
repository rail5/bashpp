/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class ProcessSubstitution : public ASTNode {
	protected:
		AST::Token<std::string> m_SUBSTITUTIONSTART;
	public:
		constexpr ProcessSubstitution() : ASTNode(bpp::AST::NodeType::ProcessSubstitution) {}

		const AST::Token<std::string>& SUBSTITUTIONSTART() const {
			return m_SUBSTITUTIONSTART;
		}
		void setSubstitutionStart(const AST::Token<std::string>& substitutionStart) {
			m_SUBSTITUTIONSTART = substitutionStart;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ProcessSubstitution\n"
				<< indent << "  " << m_SUBSTITUTIONSTART;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "))" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
