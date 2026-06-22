/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class BashIfElseBranch : public ASTNode {
	protected:
		bool m_hasCondition = false;
	public:
		constexpr BashIfElseBranch() : ASTNode(bpp::AST::NodeType::BashIfElseBranch) {}

		bool hasCondition() const {
			return m_hasCondition;
		}
		void setHasCondition(bool hasCondition) {
			m_hasCondition = hasCondition;
		}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashIfElseBranch";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
