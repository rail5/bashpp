#include "ASTNode.h"

namespace AST {

AST::NodeType ASTNode::getType() const {
	return type;
}

void ASTNode::addChild(const std::shared_ptr<ASTNode>& child) {
	children.push_back(child);
}

void ASTNode::addChildren(const std::vector<std::shared_ptr<ASTNode>>& childs) {
	children.insert(children.end(), childs.begin(), childs.end());
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

void ASTNode::setText(const std::string& txt) {
	text = txt;
}

const std::string& ASTNode::getText() const {
	return text;
}

void ASTNode::clear() {
	children.clear();
	text.clear();
	position = FilePosition{};
}

} // namespace AST
