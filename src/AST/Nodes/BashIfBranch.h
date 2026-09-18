/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class BashIfBranch : public ASTNode {
	protected:
		bool m_isRootBranch = false;
		bool m_hasCondition = false;
	public:
		constexpr BashIfBranch() : ASTNode(bpp::AST::NodeType::BashIfBranch) {}

		bool isRootBranch() const {
			return m_isRootBranch;
		}
		void setIsRootBranch(bool isRoot) {
			m_isRootBranch = isRoot;
		}

		bool hasCondition() const {
			return m_hasCondition;
		}
		void setHasCondition(bool hasCondition) {
			m_hasCondition = hasCondition;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashIfBranch";
			if (m_isRootBranch) os << " [root]";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
