/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#endif

#include <AST/NodeTypes.h>
#include <AST/Position.h>
#include <AST/Token.h>

#include <debug_helpers.h>

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
		void setPosition(std::uint32_t line, std::uint32_t column);
		const bpp::AST::FilePosition& getPosition() const;
		void setEndPosition(const bpp::AST::FilePosition& pos);
		void setEndPosition(std::uint32_t line, std::uint32_t column);
		const bpp::AST::FilePosition& getEndPosition() const;

		std::uint32_t getLine() const;
		std::uint32_t getCharPositionInLine() const;
		
		std::shared_ptr<ASTNode> getChildAt(std::size_t index) const;
		std::shared_ptr<ASTNode> getFirstChild() const;
		std::shared_ptr<ASTNode> getLastChild() const;
		std::size_t getChildrenCount() const;

		void clear();
		void clearChildren();

		PRETTYPRINT_HELPERS(ASTNode)
};

} // namespace bpp::AST
