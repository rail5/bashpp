/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class ValueAssignment : public ASTNode {
	protected:
		AST::Token<std::string> m_OPERATOR;
	public:
		constexpr ValueAssignment() : ASTNode(bpp::AST::NodeType::ValueAssignment) {}

		const AST::Token<std::string>& OPERATOR() const {
			return m_OPERATOR;
		}

		void setOperator(const AST::Token<std::string>& op) {
			m_OPERATOR = op;
		}

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ValueAssignment " << m_OPERATOR;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace bpp::AST
