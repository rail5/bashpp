/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class ObjectInstantiation : public ASTNode {
	protected:
		AST::Token<std::string> m_TYPE;
		AST::Token<std::string> m_IDENTIFIER;
		bool m_is_pointer = false;
	public:
		constexpr ObjectInstantiation() : ASTNode(bpp::AST::NodeType::ObjectInstantiation) {}

		void setType(const AST::Token<std::string>& type) {
			m_TYPE = type;
		}

		const AST::Token<std::string>& TYPE() const {
			return m_TYPE;
		}

		void setIdentifier(const AST::Token<std::string>& identifier) {
			m_IDENTIFIER = identifier;
		}

		const AST::Token<std::string>& IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		void setIsPointer(bool is_pointer) {
			m_is_pointer = is_pointer;
		}

		bool isPointer() const {
			return m_is_pointer;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ObjectInstantiation\n"
				<< indent << "  @" << m_TYPE;
			if (m_is_pointer) os << "*";
			os << " " << m_IDENTIFIER;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
