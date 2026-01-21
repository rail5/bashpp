/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class Connective : public ASTNode {
	public:
		enum class ConnectiveType {
			AND,
			OR
		};
	protected:
		ConnectiveType m_TYPE;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::Connective;
		constexpr AST::NodeType getType() const override { return static_type; }

		ConnectiveType TYPE() const {
			return m_TYPE;
		}
		void setType(ConnectiveType type) {
			m_TYPE = type;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(Connective "
				<< (m_TYPE == ConnectiveType::AND ? "&&" : "||")
				<< ")";
			return os;
		}
};

} // namespace AST
