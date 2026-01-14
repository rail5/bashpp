#pragma once

#include "../ASTNode.h"

namespace AST {

class RawText : public ASTNode {
	protected:
		std::string m_TEXT;
	public:
		RawText() {
			type = AST::NodeType::RawText;
		}

		const std::string& TEXT() const {
			return m_TEXT;
		}
		void setText(const std::string& text) {
			m_TEXT = text;
		}
		void appendText(const std::string& text) {
			m_TEXT += text;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(RawText\n"
				<< indent << "  " << m_TEXT << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
