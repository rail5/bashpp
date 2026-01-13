#include "ASTNode.h"

namespace AST {

AST::NodeType ASTNode::getType() const {
	return type;
}

void ASTNode::addChild(const std::shared_ptr<ASTNode>& child) {
	children.push_back(child);
}

const std::vector<std::shared_ptr<ASTNode>>& ASTNode::getChildren() const {
	return children;
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
