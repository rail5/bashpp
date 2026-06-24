/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class BashArithmeticSubstitution : public ASTNode {
	public:
		constexpr BashArithmeticSubstitution() : ASTNode(bpp::AST::NodeType::BashArithmeticSubstitution) {}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashArithmeticSubstitution $((";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")) )" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
