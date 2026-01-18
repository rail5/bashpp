/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

class HeredocBody : public StringType {
	protected:
		AST::Token<std::string> m_DELIMITER;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::HeredocBody;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& DELIMITER() const {
			return m_DELIMITER;
		}
		void setDelimiter(const AST::Token<std::string>& delimiter) {
			m_DELIMITER = delimiter;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(HeredocBody <<[-]" << m_DELIMITER;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << m_DELIMITER << ")" << std::flush;
			return os;
		}
};

} // namespace AST
