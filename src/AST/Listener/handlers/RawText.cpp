/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>

namespace bpp::AST {

template <>
void Listener::enter(RawText* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when entering RawText node");
	std::shared_ptr<bpp::IR::CodeEntity> current_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_entity->add(node->TEXT().getValue());
}

template <>
void Listener::exit(RawText* node) {}

} // namespace bpp::AST
