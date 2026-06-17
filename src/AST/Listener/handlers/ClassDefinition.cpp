/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

#include <iostream>

namespace bpp::AST {

template <>
void Listener::enter(ClassDefinition* node) {
	std::cout << "Entered class definition node" << std::endl;
	auto class_entity = std::make_shared<bpp::IR::Class>(node->CLASSNAME());
	entity_stack.push(class_entity);
}

template <>
void Listener::exit(ClassDefinition* node) {
	std::cout << "Exited class definition node" << std::endl;
	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting ClassDefinition node");
	std::shared_ptr<bpp::IR::Class> class_entity = std::static_pointer_cast<bpp::IR::Class>(entity_stack.top());
	entity_stack.pop();
	program->add(class_entity);
}

} // namespace bpp::AST
