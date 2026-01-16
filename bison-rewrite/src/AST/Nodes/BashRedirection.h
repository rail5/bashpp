#pragma once

#include "../ASTNode.h"
#include "StringType.h"

namespace AST {

class BashRedirection : public StringType {
	protected:
		std::string m_OPERATOR;
	public:
		BashRedirection() {
			type = AST::NodeType::BashRedirection;
		}

		const std::string& OPERATOR() const {
			return m_OPERATOR;
		}
		void setOperator(const std::string& op) {
			m_OPERATOR = op;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(BashRedirection " << m_OPERATOR;
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
