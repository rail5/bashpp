/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

/**
 * @brief Specifically, this represents braced variable references such as `${var}` or `${var:-default}`.
 * Simple variable references such as `$var` are lexed as single tokens and are not represented as AST nodes.
 *
 * TEXT() contains the text of the variable reference, as well as any *preceding* modifiers (such as indirection)
 *
 * E.g., in `${!var:-default}`, TEXT() would contain `!var`, and the `:-default` part would be represented as a child node of this BashVariable node.
 */
class BashVariable : public ASTNode {
	protected:
		AST::Token<std::string> m_TEXT;
	public:
		constexpr BashVariable() : ASTNode(bpp::AST::NodeType::BashVariable) {}

		const AST::Token<std::string>& TEXT() const {
			return m_TEXT;
		}

		void setText(const AST::Token<std::string>& text) {
			m_TEXT = text;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashVariable\n"
				<< indent << "  ${" << m_TEXT;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "})" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
