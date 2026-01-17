/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <iostream>


#include "NodeTypes.h"
#include "Position.h"
#include "Token.h"

namespace AST {

/**
 * @class ASTNode
 * @brief The base class for all non-terminal nodes in the Bash++ AST.
 * Each ASTNode contains information about its type, children, and position in the source code.
 * 
 */
class ASTNode {
	protected:
		AST::NodeType type = AST::NodeType::ERROR_TYPE;
		std::vector<std::shared_ptr<ASTNode>> children;
		AST::FilePosition position;
		AST::FilePosition end_position;

	public:
		ASTNode() = default;
		
		AST::NodeType getType() const;

		void addChild(const std::shared_ptr<ASTNode>& child);
		void addChildren(const std::vector<std::shared_ptr<ASTNode>>& childs);
		const std::vector<std::shared_ptr<ASTNode>>& getChildren() const;
		void setPosition(const AST::FilePosition& pos);
		void setPosition(uint32_t line, uint32_t column);
		const AST::FilePosition& getPosition() const;
		void setEndPosition(const AST::FilePosition& pos);
		void setEndPosition(uint32_t line, uint32_t column);
		const AST::FilePosition& getEndPosition() const;

		uint32_t getLine() const;
		uint32_t getCharPositionInLine() const;
		
		std::shared_ptr<ASTNode> getChildAt(size_t index) const;
		std::shared_ptr<ASTNode> getFirstChild() const;
		std::shared_ptr<ASTNode> getLastChild() const;
		size_t getChildrenCount() const;

		void clear();
		void clearChildren();

		virtual std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const = 0;
		friend std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
			return node.prettyPrint(os, 0);
		}
};

} // namespace AST
