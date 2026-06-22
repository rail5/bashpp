/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class BashFunction : public ASTNode {
	protected:
		AST::Token<std::string> m_NAME;
	public:
		constexpr BashFunction() : ASTNode(bpp::AST::NodeType::BashFunction) {}

		const AST::Token<std::string>& NAME() const {
			return m_NAME;
		}
		void setName(const AST::Token<std::string>& name) {
			m_NAME = name;
		}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashFunction function " << m_NAME.getValue() << "()";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
