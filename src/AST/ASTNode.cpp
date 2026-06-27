/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/ASTNode.h>
#include <AST/Nodes/RawText.h>
#include <AST/Position.h>

namespace bpp::AST {

/**
 * @brief Add a child node to this AST node.
 * This function also:
 *  1. Filters out null child nodes
 *  2. Merges consecutive RawText nodes into a single RawText node to optimize the AST structure.
 * 
 * @param child The child AST node to add.
 */
void ASTNode::addChild(const std::shared_ptr<ASTNode>& child) {
	if (child == nullptr) return;
	if (child->getType() == bpp::AST::NodeType::RawText
		&& children.size() > 0
		&& children.back()->getType() == bpp::AST::NodeType::RawText
	) {
		// Merge with last RawText child
		auto lastRawText = std::static_pointer_cast<bpp::AST::RawText>(children.back());
		auto newRawText = std::static_pointer_cast<bpp::AST::RawText>(child);
		lastRawText->appendText(newRawText->TEXT());
		return;
	}
	children.push_back(child);
}

/**
 * @brief Add a vector of child nodes to this AST node.
 * This function also:
 *  1. Filters out null child nodes
 *  2. Merges consecutive RawText nodes into a single RawText node to optimize the AST structure.
 * 
 * @param childs The vector of child AST nodes to add.
 */
void ASTNode::addChildren(const std::vector<std::shared_ptr<ASTNode>>& childs) {
	if (childs.empty()) return;

	children.reserve(children.size() + childs.size());

	auto lastRawText = std::dynamic_pointer_cast<bpp::AST::RawText>(children.empty() ? nullptr : children.back());

	for (const auto& child : childs) {
		if (child == nullptr) continue;

		if (child->getType() != bpp::AST::NodeType::RawText) {
			children.push_back(child);
			lastRawText = nullptr;
			continue;
		}

		// Child is RawText
		if (lastRawText != nullptr) {
			// Merge with last RawText child
			auto newRawText = std::static_pointer_cast<bpp::AST::RawText>(child);
			lastRawText->appendText(newRawText->TEXT());
		} else {
			children.push_back(child);
			lastRawText = std::static_pointer_cast<bpp::AST::RawText>(child);
		}
	}
}

const std::vector<std::shared_ptr<bpp::AST::ASTNode>>& ASTNode::getChildren() const {
	return children;
}

void ASTNode::setPosition(const bpp::AST::FilePosition& pos) {
	position = pos;
}

void ASTNode::setPosition(std::uint32_t line, std::uint32_t column) {
	position.line = line;
	position.column = column;
}

const bpp::AST::FilePosition& ASTNode::getPosition() const {
	return position;
}

void ASTNode::setEndPosition(const bpp::AST::FilePosition& pos) {
	end_position = pos;
}

void ASTNode::setEndPosition(std::uint32_t line, std::uint32_t column) {
	end_position.line = line;
	end_position.column = column;
}

const bpp::AST::FilePosition& ASTNode::getEndPosition() const {
	if (end_position.line == 0 && end_position.column == 0) {
		// If end_position is not set, return position instead
		return position;
	}
	return end_position;
}

std::uint32_t ASTNode::getLine() const {
	return position.line;
}

std::uint32_t ASTNode::getCharPositionInLine() const {
	return position.column;
}

std::shared_ptr<ASTNode> ASTNode::getChildAt(size_t index) const {
	if (index < children.size()) {
		return children[index];
	}
	return nullptr;
}

std::shared_ptr<ASTNode> ASTNode::getFirstChild() const {
	if (!children.empty()) {
		return children.front();
	}
	return nullptr;
}

std::shared_ptr<ASTNode> ASTNode::getLastChild() const {
	if (!children.empty()) {
		return children.back();
	}
	return nullptr;
}

size_t ASTNode::getChildrenCount() const {
	return children.size();
}

void ASTNode::clear() {
	children.clear();
	position = FilePosition{};
}

void ASTNode::clearChildren() {
	children.clear();
}

} // namespace bpp::AST
