#pragma once

#include "../ASTNode.h"

namespace AST {

class ProcessSubstitution : public ASTNode {
	protected:
		std::string m_SUBSTITUTIONSTART;
	public:
		ProcessSubstitution() {
			type = AST::NodeType::ProcessSubstitution;
		}

		const std::string& SUBSTITUTIONSTART() const {
			return m_SUBSTITUTIONSTART;
		}
		void setSubstitutionStart(const std::string& substitutionStart) {
			m_SUBSTITUTIONSTART = substitutionStart;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
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
