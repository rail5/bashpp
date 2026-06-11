/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <iostream>

#include <AST/NodeTypes.h>
#include <AST/Position.h>
#include <AST/Token.h>

#define PRETTYPRINT_INDENTATION_AMOUNT 4

namespace bpp::AST {

/**
 * @class ASTNode
 * @brief The base class for all non-terminal nodes in the Bash++ AST.
 * Each ASTNode contains information about its type, children, and position in the source code.
 * 
 */
class ASTNode {
	private:
		bpp::AST::NodeType _type = bpp::AST::NodeType::ERROR_TYPE;
	protected:
		std::vector<std::shared_ptr<bpp::AST::ASTNode>> children;
		bpp::AST::FilePosition position;
		bpp::AST::FilePosition end_position;

	public:
		ASTNode() = default;
		constexpr explicit ASTNode(bpp::AST::NodeType type) : _type(type) {}
		virtual ~ASTNode() = default;

		ASTNode(const ASTNode& other) = default;
		ASTNode& operator=(const ASTNode& other) = default;
		ASTNode(ASTNode&& other) noexcept = default;
		ASTNode& operator=(ASTNode&& other) noexcept = default;
		
		constexpr bpp::AST::NodeType getType() const { return _type; }

		void addChild(const std::shared_ptr<ASTNode>& child);
		void addChildren(const std::vector<std::shared_ptr<ASTNode>>& childs);
		const std::vector<std::shared_ptr<ASTNode>>& getChildren() const;
		void setPosition(const bpp::AST::FilePosition& pos);
		void setPosition(uint32_t line, uint32_t column);
		const bpp::AST::FilePosition& getPosition() const;
		void setEndPosition(const bpp::AST::FilePosition& pos);
		void setEndPosition(uint32_t line, uint32_t column);
		const bpp::AST::FilePosition& getEndPosition() const;

		uint32_t getLine() const;
		uint32_t getCharPositionInLine() const;
		
		std::shared_ptr<ASTNode> getChildAt(size_t index) const;
		std::shared_ptr<ASTNode> getFirstChild() const;
		std::shared_ptr<ASTNode> getLastChild() const;
		size_t getChildrenCount() const;

		void clear();
		void clearChildren();

		virtual std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const = 0;
		friend std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
			return node.prettyPrint(os, 0);
		}
};

} // namespace bpp::AST
