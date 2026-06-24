/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace bpp::AST {

/**
 * @class BashCasePatternHeader
 * @brief Represents the 'header' of a case pattern, i.e., the pattern to match against.
 * 
 */
class BashCasePatternHeader : public StringType {
	public:
		constexpr BashCasePatternHeader() : StringType(bpp::AST::NodeType::BashCasePatternHeader) {}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashCasePatternHeader";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "')')" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
