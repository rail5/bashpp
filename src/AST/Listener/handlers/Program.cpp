/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Program.h>

namespace bpp::AST {

template <>
void Listener::enter(Program* /*node*/) {
	auto program = std::make_shared<bpp::IR::Program>();
	entity_stack.push(program);
	this->program = program;
}

template <>
void Listener::exit(Program* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program when exiting Program node");
	std::shared_ptr<bpp::IR::Program> program = std::static_pointer_cast<bpp::IR::Program>(entity_stack.top());
	entity_stack.pop();
	bpp_assert(entity_stack.empty(), "Entity stack is not empty after popping Program entity");
}

} // namespace bpp::AST
