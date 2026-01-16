#pragma once

#include "../ASTNode.h"

namespace AST {

class PrimitiveAssignment : public ASTNode {
	protected:
		std::string m_IDENTIFIER;
	public:
		PrimitiveAssignment() {
			type = AST::NodeType::PrimitiveAssignment;
		}

		const std::string& IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		void setIdentifier(const std::string& identifier) {
			m_IDENTIFIER = identifier;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(PrimitiveAssignment\n"
				<< indent << "  " << m_IDENTIFIER << "=\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
