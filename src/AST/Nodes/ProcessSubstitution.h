/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class ProcessSubstitution : public ASTNode {
	protected:
		AST::Token<std::string> m_SUBSTITUTIONSTART;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::ProcessSubstitution;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& SUBSTITUTIONSTART() const {
			return m_SUBSTITUTIONSTART;
		}
		void setSubstitutionStart(const AST::Token<std::string>& substitutionStart) {
			m_SUBSTITUTIONSTART = substitutionStart;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ProcessSubstitution\n"
				<< indent << "  " << m_SUBSTITUTIONSTART;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "))" << std::flush;
			return os;
		}
};

} // namespace AST
