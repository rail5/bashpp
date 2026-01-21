/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"
#include <optional>

namespace AST {

class ClassDefinition : public ASTNode {
	protected:
		AST::Token<std::string> m_CLASSNAME;
		std::optional<AST::Token<std::string>> m_PARENTCLASSNAME;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::ClassDefinition;
		constexpr AST::NodeType getType() const override { return static_type; }

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

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
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
		}
};

} // namespace AST
