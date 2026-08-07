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
}

template <>
void Listener::exit(DestructorDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting DestructorDefinition node");
	auto destructor = std::static_pointer_cast<bpp::IR::Method>(entity_stack.top());

	auto current_class = destructor->get_containing_class().lock();
	bpp_assert(current_class != nullptr, "Destructor's containing class is null when exiting DestructorDefinition node");
	auto parent_class = current_class->get_parent_class();
	if (parent_class) {
		auto parent_destructor = parent_class->get_method_UNSAFE("__destructor");
		if (parent_destructor) {
			// FIXME(@rail5): Call parent destructor at the end of the child destructor.

			parent_destructor->mark_referenced_by(destructor);
		}
	}

	entity_stack.pop();
}

} // namespace bpp::AST
