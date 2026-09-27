/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/ASTNode.h>
#include <AST/Nodes/RawText.h>
#include <AST/Position.h>

namespace bpp::AST {

// We use getType() to check the types of ASTNodes, we can safely use static_cast here
// NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)

void ASTNode::addChild(std::unique_ptr<ASTNode>&& child) {
	if (child == nullptr) return;
	if (child->getType() == bpp::AST::NodeType::RawText
		&& !children.empty()
		&& children.back()->getType() == bpp::AST::NodeType::RawText
	) {
		// Merge with last RawText child
		auto* lastRawText = static_cast<bpp::AST::RawText*>(children.back().get());
		auto* newRawText = static_cast<bpp::AST::RawText*>(child.get());
		lastRawText->appendText(newRawText->TEXT());
		return;
	}
	children.push_back(std::move(child));
}

void ASTNode::addChildren(std::vector<std::unique_ptr<ASTNode>>&& childs) {
	if (childs.empty()) return;

	children.reserve(children.size() + childs.size());

	bpp::AST::RawText* lastRawText = nullptr;
	if (!children.empty() && children.back()->getType() == bpp::AST::NodeType::RawText) {
		lastRawText = static_cast<bpp::AST::RawText*>(children.back().get());
	}

	for (auto&& child : childs) {
		if (child == nullptr) continue;

		if (child->getType() != bpp::AST::NodeType::RawText) {
			children.push_back(std::move(child));
			lastRawText = nullptr;
			continue;
		}

		// Child is RawText
		if (lastRawText != nullptr) {
			// Merge with last RawText child
			auto* newRawText = static_cast<bpp::AST::RawText*>(child.get());
			lastRawText->appendText(newRawText->TEXT());
		} else {
			lastRawText = static_cast<bpp::AST::RawText*>(child.get());
			children.push_back(std::move(child));
		}
	}
}

std::span<const std::unique_ptr<bpp::AST::ASTNode>> ASTNode::getChildren() const {
	return {children.data(), children.size()};
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

ASTNode* ASTNode::getChildAt(std::size_t index) const {
	if (index < children.size()) {
		return children[index].get();
	}
	return nullptr;
}

ASTNode* ASTNode::getFirstChild() const {
	if (!children.empty()) {
		return children.front().get();
	}
	return nullptr;
}

ASTNode* ASTNode::getLastChild() const {
	if (!children.empty()) {
		return children.back().get();
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

// NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

} // namespace bpp::AST
