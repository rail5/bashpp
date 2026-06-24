/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class RawText : public ASTNode {
	protected:
		AST::Token<std::string> m_TEXT;
		std::string getEscapedText() const {
			std::string escaped;
			for (char c : m_TEXT.getValue()) {
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
		constexpr RawText() : ASTNode(bpp::AST::NodeType::RawText) {}

		const AST::Token<std::string>& TEXT() const {
			return m_TEXT;
		}
		void setText(const AST::Token<std::string>& text) {
			m_TEXT = text;
		}
		void appendText(const std::string& text) {
			m_TEXT += text;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(RawText " << getEscapedText();
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
