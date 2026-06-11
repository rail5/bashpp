/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Listener.h"

namespace bpp::AST {

void Listener::walk(std::shared_ptr<bpp::AST::ASTNode> node) {
	try {
		switch (node->getType()) {
			#define AST_LISTENER_NODE_CASE(node_type) \
				case bpp::AST::NodeType::node_type: \
					enter(std::static_pointer_cast<bpp::AST::node_type>(node)); \
					for (const auto& child : node->getChildren()) { \
						walk(child); \
					} \
					exit(std::static_pointer_cast<bpp::AST::node_type>(node)); \
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
#undef AST_LISTENER_NODE_LIST

} // namespace bpp::AST
