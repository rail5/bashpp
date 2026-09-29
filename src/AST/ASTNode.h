/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <span>

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
		std::vector<std::unique_ptr<bpp::AST::ASTNode>> children;
		bpp::AST::FilePosition position;
		bpp::AST::FilePosition end_position;

	public:
		ASTNode() = default;
		constexpr explicit ASTNode(bpp::AST::NodeType type) : _type(type) {}
		virtual ~ASTNode() = default;

		ASTNode(const ASTNode& other) = delete;
		ASTNode& operator=(const ASTNode& other) = delete;
		ASTNode(ASTNode&& other) noexcept = default;
		ASTNode& operator=(ASTNode&& other) noexcept = default;

		constexpr bpp::AST::NodeType getType() const { return _type; }

		/**
		 * @brief Add a child node to this AST node.
		 * This function also:
		 *  1. Filters out null child nodes
		 *  2. Merges consecutive RawText nodes into a single RawText node to optimize the AST structure.
		 * 
		 * @param child The child AST node to add.
		 */
		void addChild(std::unique_ptr<ASTNode>&& child);

		/**
		 * @brief Add a vector of child nodes to this AST node.
		 * This function also:
		 *  1. Filters out null child nodes
		 *  2. Merges consecutive RawText nodes into a single RawText node to optimize the AST structure.
		 * 
		 * @param childs The vector of child AST nodes to add.
		 */
		void addChildren(std::vector<std::unique_ptr<ASTNode>>&& childs);
		std::span<const std::unique_ptr<ASTNode>> getChildren() const;
		std::vector<std::unique_ptr<ASTNode>> releaseChildren() { return std::move(children); }
		void setPosition(const bpp::AST::FilePosition& pos);
		void setPosition(std::uint32_t line, std::uint32_t column);
		const bpp::AST::FilePosition& getPosition() const;
		void setEndPosition(const bpp::AST::FilePosition& pos);
		void setEndPosition(std::uint32_t line, std::uint32_t column);
		const bpp::AST::FilePosition& getEndPosition() const;

		std::uint32_t getLine() const;
		std::uint32_t getCharPositionInLine() const;

		ASTNode* getChildAt(std::size_t index) const;
		ASTNode* getFirstChild() const;
		ASTNode* getLastChild() const;
		std::size_t getChildrenCount() const;

		void clear();
		void clearChildren();

		PRETTYPRINT_HELPERS(ASTNode)
};

/**
 * @brief Casts a unique_ptr of ASTNode to a unique_ptr of a derived type T.
 * This function does not perform any type checking. It should only be used when the caller is certain that the type is in fact T.
 * The inputted unique_ptr will be released, and ownership of the pointed-to object will be transferred to the returned unique_ptr.
 * @tparam T The derived type to cast to.
 * @param p The unique_ptr of ASTNode to cast.
 * @return std::unique_ptr<T> The casted unique_ptr of type T.
 */
template <typename T>
std::unique_ptr<T> static_uniqueptr_cast(std::unique_ptr<bpp::AST::ASTNode> p) {
	return std::unique_ptr<T>(static_cast<T*>(p.release()));
}

} // namespace bpp::AST
