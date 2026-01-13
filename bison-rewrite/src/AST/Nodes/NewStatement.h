#pragma once

#include "../ASTNode.h"

namespace AST {

class NewStatement : public ASTNode {
	protected:
		std::string m_TYPE;
	public:
		NewStatement() {
			type = AST::NodeType::NewStatement;
		}

		const std::string& TYPE() const {
			return m_TYPE;
		}

		void setType(const std::string& type) {
			m_TYPE = type;
		}
};

} // namespace AST
