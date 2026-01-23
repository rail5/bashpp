/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"

namespace AST {

class Bash53NativeSupershell : public ASTNode {
	protected:
		AST::Token<std::string> m_STARTTOKEN;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::Bash53NativeSupershell;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& STARTTOKEN() const {
			return m_STARTTOKEN;
		}
		void setStartToken(const AST::Token<std::string>& start) {
			m_STARTTOKEN = start;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(Bash53NativeSupershell ${ ";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "; })" << std::flush;
			return os;
		}
};

} // namespace AST
