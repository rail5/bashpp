/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Method.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ConstructorDefinition* node) {
	auto current_class = std::dynamic_pointer_cast<bpp::IR::Class>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor definition outside of class body");

	auto constructor = std::make_shared<bpp::IR::Method>();
	constructor->inherit(current_class);
	constructor->set_name("__constructor");
	constructor->set_scope(bpp::IR::VisibilityScope::PUBLIC);

	if (!current_class->add_method(constructor)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor already defined in class '" + current_class->get_name() + "'");
	}
	constructor->set_definition_position({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	auto this_ptr = std::make_shared<bpp::IR::ThisPtr>(constructor);
	this_ptr->inherit(constructor);
	constructor->add_parameter(this_ptr);

	entity_stack.push(constructor);
	in_method = true;
}

template <>
void Listener::exit(ConstructorDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting ConstructorDefinition node");
	entity_stack.pop();
	in_method = false;
}

} // namespace bpp::AST
