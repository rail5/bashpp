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
void Listener::enter(DestructorDefinition* node) {
	auto current_class = std::dynamic_pointer_cast<bpp::IR::Class>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor definition outside of class body");

	auto destructor = std::make_shared<bpp::IR::Method>();
	destructor->inherit(current_class);
	destructor->set_name("__destructor");
	destructor->set_scope(bpp::IR::VisibilityScope::PUBLIC);
	destructor->set_is_virtual(true);

	if (!current_class->add_method(destructor)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor already defined in class '" + current_class->get_name() + "'");
	}
	destructor->set_definition_position({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	auto this_ptr = std::make_shared<bpp::IR::ThisPtr>(destructor);
	this_ptr->inherit(destructor);
	destructor->add_parameter(this_ptr);

	entity_stack.push(destructor);
	in_method = true;
}

template <>
void Listener::exit(DestructorDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting DestructorDefinition node");
	entity_stack.pop();
	in_method = false;
}

} // namespace bpp::AST
