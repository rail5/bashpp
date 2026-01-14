#pragma once

#include "../ASTNode.h"

namespace AST {

class ObjectInstantiation : public ASTNode {
	protected:
		std::string m_TYPE;
		std::string m_IDENTIFIER;
	public:
		ObjectInstantiation() {
			type = AST::NodeType::ObjectInstantiation;
		}

		void setType(const std::string& type) {
			m_TYPE = type;
		}

		std::string TYPE() const {
			return m_TYPE;
		}

		void setIdentifier(const std::string& identifier) {
			m_IDENTIFIER = identifier;
		}

		std::string IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(ObjectInstantiation\n"
				<< indent << "  @" << m_TYPE << " " << m_IDENTIFIER << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
