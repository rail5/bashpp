/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <optional>

namespace bpp::AST {

class DynamicCastTarget : public ASTNode {
	protected:
		std::optional<AST::Token<std::string>> m_TARGETTYPE;
	public:
		constexpr DynamicCastTarget() : ASTNode(bpp::AST::NodeType::DynamicCastTarget) {}

		void setTargetType(const AST::Token<std::string>& target_type) {
			m_TARGETTYPE = target_type;
		}
		const std::optional<AST::Token<std::string>>& TARGETTYPE() const {
			return m_TARGETTYPE;
		}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(DynamicCastTarget"
				<< (m_TARGETTYPE.has_value() ? " " + m_TARGETTYPE->getValue() : "");
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
