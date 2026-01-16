#pragma once

#include "../ASTNode.h"

namespace AST {

class BashVariable : public ASTNode {
	protected:
		std::string m_TEXT;
	public:
		BashVariable() {
			type = AST::NodeType::BashVariable;
		}

		const std::string& TEXT() const {
			return m_TEXT;
		}

		void setText(const std::string& text) {
			m_TEXT = text;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashVariable\n"
				<< indent << "  ${" << m_TEXT;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "})" << std::flush;
			return os;
		}
};

} // namespace AST
