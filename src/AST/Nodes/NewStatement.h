/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class NewStatement : public ASTNode {
	protected:
		AST::Token<std::string> m_TYPE;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::NewStatement;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& TYPE() const {
			return m_TYPE;
		}

		void setType(const AST::Token<std::string>& type) {
			m_TYPE = type;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(NewStatement\n"
				<< indent << "  @new " << m_TYPE;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
