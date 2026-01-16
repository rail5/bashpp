/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class BashSelectStatement : public ASTNode {
	protected:
		std::string m_VARIABLE;
	public:
		BashSelectStatement() {
			type = AST::NodeType::BashSelectStatement;
		}

		const std::string& VARIABLE() const {
			return m_VARIABLE;
		}

		void setVariable(const std::string& variable) {
			m_VARIABLE = variable;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashSelectStatement\n"
				<< indent << "  select " << m_VARIABLE;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
