#pragma once

#include "../ASTNode.h"

namespace AST {

class ValueAssignment : public ASTNode {
	protected:
		std::string m_OPERATOR;
	public:
		ValueAssignment() {
			type = AST::NodeType::ValueAssignment;
		}

		const std::string& OPERATOR() const {
			return m_OPERATOR;
		}

		void setOperator(const std::string& op) {
			m_OPERATOR = op;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(ValueAssignment\n"
				<< indent << "  " << m_OPERATOR << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
