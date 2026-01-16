#pragma once

#include "../ASTNode.h"

namespace AST {

class RawText : public ASTNode {
	protected:
		std::string m_TEXT;
		const std::string getEscapedText() const {
			std::string escaped;
			for (char c : m_TEXT) {
				switch (c) {
					case '\n': escaped += "\\n"; break;
					case '\t': escaped += "\\t"; break;
					case '\r': escaped += "\\r"; break;
					default: escaped += c; break;
				}
			}
			return escaped;
		}
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
			os << indent << "(RawText " << getEscapedText();
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
