/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BaseListener.h"
#include <stdexcept>

void AST::BaseListener::walk(std::shared_ptr<AST::ASTNode> node) {
	if (node == nullptr) return;

	std::function<void(std::shared_ptr<AST::ASTNode>)> enterFunc = nullptr;
	std::function<void(std::shared_ptr<AST::ASTNode>)> exitFunc = nullptr;

	auto it = enterExitMap.find(node->getType());
	if (it != enterExitMap.end()) {
		enterFunc = it->second.first;
		exitFunc = it->second.second;
	} else {
		throw std::runtime_error("No enter/exit functions defined for node type in BaseListener.");
	}

	// Enter this node, walk children, then exit this node
	enterFunc(node);
	for (const auto& child : node->getChildren()) {
		walk(child);
	}
	exitFunc(node);
}
