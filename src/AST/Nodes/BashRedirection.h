/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace bpp::AST {

class BashRedirection : public StringType {
	protected:
		AST::Token<std::string> m_OPERATOR;
	public:
		constexpr BashRedirection() : StringType(bpp::AST::NodeType::BashRedirection) {}

		const AST::Token<std::string>& OPERATOR() const {
			return m_OPERATOR;
		}
		void setOperator(const AST::Token<std::string>& op) {
			m_OPERATOR = op;
		}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashRedirection " << m_OPERATOR;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
