/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class ArrayAssignment : public ASTNode {
	public:
		constexpr ArrayAssignment() : ASTNode(bpp::AST::NodeType::ArrayAssignment) {}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ArrayAssignment (";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "))" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
