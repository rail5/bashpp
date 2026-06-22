/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace bpp::AST {

class ParameterExpansion : public StringType {
	protected:
		AST::Token<std::string> m_EXPANSIONBEGIN;
	public:
		constexpr ParameterExpansion() : StringType(bpp::AST::NodeType::ParameterExpansion) {}

		const AST::Token<std::string>& EXPANSIONBEGIN() const {
			return m_EXPANSIONBEGIN;
		}

		void setExpansionBegin(const AST::Token<std::string>& expansionBegin) {
			m_EXPANSIONBEGIN = expansionBegin;
		}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ParameterExpansion\n"
				<< indent << "  " << m_EXPANSIONBEGIN;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
