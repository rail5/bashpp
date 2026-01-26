/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>

namespace AST {

class ParameterExpansion : public StringType {
	protected:
		AST::Token<std::string> m_EXPANSIONBEGIN;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::ParameterExpansion;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& EXPANSIONBEGIN() const {
			return m_EXPANSIONBEGIN;
		}

		void setExpansionBegin(const AST::Token<std::string>& expansionBegin) {
			m_EXPANSIONBEGIN = expansionBegin;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ParameterExpansion\n"
				<< indent << "  " << m_EXPANSIONBEGIN;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
