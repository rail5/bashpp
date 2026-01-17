/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BaseListener.h"
#include "../../internal_error.h"

/**
 * @brief Walk the AST starting from the given node, calling enter and exit methods for each node.
 * This method dispatches to the appropriate enter and exit methods based on the node type,
 * walking the tree in a depth-first manner.
 * 
 * @param node The AST node to start walking from.
 */
void AST::BaseListener::walk(std::shared_ptr<AST::ASTNode> node) {
	if (node == nullptr) return;

	std::function<void(std::shared_ptr<AST::ASTNode>)> enterFunc = nullptr;
	std::function<void(std::shared_ptr<AST::ASTNode>)> exitFunc = nullptr;

	auto it = enterExitMap.find(node->getType());
	if (it != enterExitMap.end()) {
		enterFunc = it->second.first;
		exitFunc = it->second.second;
	} else {
		throw internal_error("No enter/exit functions defined for node type "
			+ std::to_string(static_cast<int>(node->getType()))
			+ " in BaseListener.");
	}

	// Enter this node, walk children, then exit this node
	enterFunc(node);
	for (const auto& child : node->getChildren()) {
		walk(child);
	}
	exitFunc(node);
}
