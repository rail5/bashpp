/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class Connective : public ASTNode {
	public:
		enum class ConnectiveType : std::uint8_t {
			AND,
			OR
		};
	protected:
		ConnectiveType m_TYPE;
	public:
		constexpr Connective() : ASTNode(bpp::AST::NodeType::Connective) {}

		ConnectiveType TYPE() const {
			return m_TYPE;
		}
		void setType(ConnectiveType type) {
			m_TYPE = type;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(Connective "
				<< (m_TYPE == ConnectiveType::AND ? "&&" : "||")
				<< ")";
			return os;
		})
};

} // namespace bpp::AST
