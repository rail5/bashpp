#pragma once

#include "../ASTNode.h"

namespace AST {

class PointerDeclaration : public ASTNode {
	protected:
		std::string m_TYPE;
		std::string m_IDENTIFIER;
	public:
		PointerDeclaration() {
			type = AST::NodeType::PointerDeclaration;
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
			os << indent << "(PointerDeclaration\n"
				<< indent << "  @" << m_TYPE << "* " << m_IDENTIFIER;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
