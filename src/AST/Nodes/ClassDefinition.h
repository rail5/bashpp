/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <optional>

namespace bpp::AST {

class ClassDefinition : public ASTNode {
	protected:
		AST::Token<std::string> m_CLASSNAME;
		std::optional<AST::Token<std::string>> m_PARENTCLASSNAME;
	public:
		constexpr ClassDefinition() : ASTNode(bpp::AST::NodeType::ClassDefinition) {}

		const AST::Token<std::string>& CLASSNAME() const {
			return m_CLASSNAME;
		}

		void setClassName(const AST::Token<std::string>& classname) {
			m_CLASSNAME = classname;
		}

		const std::optional<AST::Token<std::string>>& PARENTCLASSNAME() const {
			return m_PARENTCLASSNAME;
		}

		void setParentClassName(const AST::Token<std::string>& parentclassname) {
			if (!parentclassname.getValue().empty()) m_PARENTCLASSNAME = parentclassname;
		}

		void clearParentClassName() {
			m_PARENTCLASSNAME = std::nullopt;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ClassDefinition\n"
				<< indent << "  @class " << m_CLASSNAME;
			if (m_PARENTCLASSNAME.has_value()) {
				os << " : " << m_PARENTCLASSNAME.value();
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
