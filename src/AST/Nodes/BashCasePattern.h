/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class BashCasePattern : public ASTNode {
	private:
		/// The terminator token for this case pattern, which can be either `;;`, `;&`, or `;;&`.
		AST::Token<std::string> m_terminator;
	public:
		constexpr BashCasePattern() : ASTNode(bpp::AST::NodeType::BashCasePattern) {}

		void setTerminator(const AST::Token<std::string>& terminator) { m_terminator = terminator; }
		const AST::Token<std::string>& TERMINATOR() const { return m_terminator; }

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashCasePattern";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << " " << TERMINATOR().getValue() << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
