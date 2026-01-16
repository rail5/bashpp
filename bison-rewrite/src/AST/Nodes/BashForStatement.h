#pragma once

#include "../ASTNode.h"

namespace AST {

class BashForStatement : public ASTNode {
	protected:
		std::string m_VARIABLE;
	public:
		BashForStatement() {
			type = AST::NodeType::BashForStatement;
		}

		const std::string& VARIABLE() const {
			return m_VARIABLE;
		}

		void setVariable(const std::string& variable) {
			m_VARIABLE = variable;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashForStatement\n"
				<< indent << "  for " << m_VARIABLE;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
