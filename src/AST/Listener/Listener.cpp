/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Listener.h"

#include <error/InternalError.h>
#include <error/SyntaxError.h>

#include <IR/entities/CodeEntity.h>

namespace bpp::AST {

void Listener::walk(bpp::AST::ASTNode* node) {
	bpp_assert(node != nullptr, "Listener::walk was given a null node pointer");
	try {
		switch (node->getType()) {
			#define AST_LISTENER_NODE_CASE(node_type) \
				case bpp::AST::NodeType::node_type: \
					enter(static_cast<node_type*>(node)); \
					for (const auto& child : node->getChildren()) { \
						walk(child.get()); \
					} \
					exit(static_cast<node_type*>(node)); \
					break;
			AST_LISTENER_NODE_LIST(AST_LISTENER_NODE_CASE)
			#undef AST_LISTENER_NODE_CASE
			default:
				throw bpp::ErrorHandling::InternalError("Listener does not know how to handle node type "
					+ std::to_string(static_cast<uint8_t>(node->getType()))
				);
		}
	} catch (const bpp::ErrorHandling::SyntaxError& e) {
		// Cancel traversal of this node and its children, but continue to traverse the rest of the tree
		this->program_has_errors = true;
		e.print();
		return;
	}
}

std::shared_ptr<bpp::IR::CodeEntity> Listener::latest_code_entity() const {
	std::stack<std::shared_ptr<bpp::IR::Entity>> temp_stack = entity_stack;
	while (!temp_stack.empty()) {
		auto top = std::dynamic_pointer_cast<bpp::IR::CodeEntity>(temp_stack.top());
		if (top) return top;
		temp_stack.pop();
	}
	return nullptr;
}

} // namespace bpp::AST
