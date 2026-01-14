#include "ASTNode.h"
#include "Nodes/RawText.h"

namespace AST {

AST::NodeType ASTNode::getType() const {
	return type;
}

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
	if (child->getType() == AST::NodeType::RawText
		&& children.size() > 0
		&& children.back()->getType() == AST::NodeType::RawText
	) {
		// Merge with last RawText child
		auto lastRawText = std::dynamic_pointer_cast<AST::RawText>(children.back());
		auto newRawText = std::dynamic_pointer_cast<AST::RawText>(child);
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

	auto lastRawText = std::dynamic_pointer_cast<AST::RawText>(children.empty() ? nullptr : children.back());

	for (const auto& child : childs) {
		if (child == nullptr) continue;

		if (child->getType() != AST::NodeType::RawText) {
			children.push_back(child);
			lastRawText = nullptr;
			continue;
		}

		// Child is RawText
		if (lastRawText != nullptr) {
			// Merge with last RawText child
			auto newRawText = std::dynamic_pointer_cast<AST::RawText>(child);
			lastRawText->appendText(newRawText->TEXT());
		} else {
			children.push_back(child);
			lastRawText = std::dynamic_pointer_cast<AST::RawText>(child);
		}
	}
}

const std::vector<std::shared_ptr<ASTNode>>& ASTNode::getChildren() const {
	return children;
}

void ASTNode::setPosition(const AST::FilePosition& pos) {
	position = pos;
}

void ASTNode::setPosition(uint32_t line, uint32_t column) {
	position.line = line;
	position.column = column;
}

const AST::FilePosition& ASTNode::getPosition() const {
	return position;
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
	text.clear();
	position = FilePosition{};
}

} // namespace AST
