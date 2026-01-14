#pragma once

#include <memory>
#include <vector>
#include <string>
#include <iostream>


#include "NodeTypes.h"
#include "Position.h"

namespace AST {

class ASTNode {
	protected:
		AST::NodeType type = AST::NodeType::ERROR_TYPE;
		std::vector<std::shared_ptr<ASTNode>> children;
		AST::FilePosition position;
		std::string text;

	public:
		ASTNode() = default;
		
		AST::NodeType getType() const;

		void addChild(const std::shared_ptr<ASTNode>& child);
		void addChildren(const std::vector<std::shared_ptr<ASTNode>>& childs);
		const std::vector<std::shared_ptr<ASTNode>>& getChildren() const;
		void setPosition(const AST::FilePosition& pos);
		void setPosition(uint32_t line, uint32_t column);
		const AST::FilePosition& getPosition() const;
		void setText(const std::string& txt);
		const std::string& getText() const;

		void clear();

		virtual std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const = 0;
		friend std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
			return node.prettyPrint(os, 0);
		}
};

} // namespace AST
