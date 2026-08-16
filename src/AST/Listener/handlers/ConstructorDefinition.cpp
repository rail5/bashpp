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

	auto new_constructor = std::make_shared<bpp::IR::Method>();
	new_constructor->inherit(current_class);
	new_constructor->set_name("__constructor");
	new_constructor->set_scope(bpp::IR::VisibilityScope::PUBLIC);

	auto res = current_class->add_method(std::move(new_constructor));
	if (!res) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor already defined in class '" + current_class->get_name() + "'");
	}
	const auto& stored_constructor = res.value();

	stored_constructor->set_definition_position({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	stored_constructor->add_parameter(current_class->get_this_ptr());

	if (auto parent_class = current_class->get_parent_class()) {
		auto parent_constructor = parent_class->get_method_UNSAFE("__constructor");
		if (parent_constructor) {
			// FIXME(@rail5): Call parent constructor at the beginning of the child constructor.

			parent_constructor->mark_referenced_by(stored_constructor);
		}
	}

	entity_stack.push(stored_constructor);
}

template <>
void Listener::exit(ConstructorDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting ConstructorDefinition node");
	entity_stack.pop();
}

} // namespace bpp::AST
