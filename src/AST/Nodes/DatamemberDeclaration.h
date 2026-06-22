/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/AccessModifier.h>
#include <optional>

namespace bpp::AST {

class DatamemberDeclaration : public ASTNode {
	protected:
		AST::Token<AccessModifier> m_ACCESSMODIFIER;
		std::optional<AST::Token<std::string>> m_TYPE;
		std::optional<AST::Token<std::string>> m_IDENTIFIER;

	public:
		constexpr DatamemberDeclaration() : ASTNode(bpp::AST::NodeType::DatamemberDeclaration) {}

		const AST::Token<AccessModifier>& ACCESSMODIFIER() const {
			return m_ACCESSMODIFIER;
		}

		void setAccessModifier(const AST::Token<AccessModifier>& accessmodifier) {
			m_ACCESSMODIFIER = accessmodifier;
		}

		const std::optional<AST::Token<std::string>>& TYPE() const {
			return m_TYPE;
		}

		void setType(const AST::Token<std::string>& type) {
			if (!type.getValue().empty()) m_TYPE = type;
		}

		void clearType() {
			m_TYPE = std::nullopt;
		}

		const std::optional<AST::Token<std::string>>& IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		void setIdentifier(const AST::Token<std::string>& identifier) {
			if (!identifier.getValue().empty()) m_IDENTIFIER = identifier;
		}

		void clearIdentifier() {
			m_IDENTIFIER = std::nullopt;
		}

		PRETTYPRINT_IMPLEMENTATION_IN_HEADER({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(DatamemberDeclaration\n"
				<< indent << "  ";
			switch (m_ACCESSMODIFIER) {
				case AccessModifier::PUBLIC:
					os << "@public ";
					break;
				case AccessModifier::PROTECTED:
					os << "@protected ";
					break;
				default: // TODO(@rail5): Is this an error case we should handle explicitly?
				case AccessModifier::PRIVATE:
					os << "@private ";
					break;
			}
			if (m_TYPE.has_value()) {
				os << "@" << m_TYPE.value() << " ";
			}
			if (m_IDENTIFIER.has_value()) {
				os << m_IDENTIFIER.value();
			}
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
