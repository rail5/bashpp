/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Listener.h"

#include <error/InternalError.h>
#include <error/Diagnostic.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/Program.h>

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
			AST_LISTENER_NODE_LIST(AST_LISTENER_NODE_CASE) // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast) The type is known, static_cast is safe.
			#undef AST_LISTENER_NODE_CASE
			default:
				throw bpp::ErrorHandling::InternalError("Listener does not know how to handle node type "
					+ std::to_string(static_cast<std::uint8_t>(node->getType()))
				);
		}
	} catch (const bpp::ErrorHandling::Diagnostic& e) {
		// Cancel traversal of this node and its children, but continue to traverse the rest of the tree
		this->program_has_errors = true;
		e.print_to_stderr();
		return;
	}
}

std::unique_ptr<bpp::IR::Program> Listener::release_program() {
	return std::move(program);
}

} // namespace bpp::AST
