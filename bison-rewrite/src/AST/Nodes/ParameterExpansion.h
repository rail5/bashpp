#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

class ParameterExpansion : public StringType {
	protected:
		std::string m_EXPANSIONBEGIN;
	public:
		ParameterExpansion() {
			type = AST::NodeType::ParameterExpansion;
		}

		const std::string& EXPANSIONBEGIN() const {
			return m_EXPANSIONBEGIN;
		}

		void setExpansionBegin(const std::string& expansionBegin) {
			m_EXPANSIONBEGIN = expansionBegin;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(ParameterExpansion\n"
				<< indent << "  " << m_EXPANSIONBEGIN << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
