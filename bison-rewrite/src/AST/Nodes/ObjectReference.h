#pragma once

#include "../ASTNode.h"

namespace AST {

class ObjectReference : public ASTNode {
	protected:
		std::string m_IDENTIFIER;
		std::vector<std::string> m_IDENTIFIERS;
		char m_HASHKEY; // '#' or '\0' if not present
		std::shared_ptr<AST::ASTNode> m_ARRAYINDEX;
};

} // namespace AST
