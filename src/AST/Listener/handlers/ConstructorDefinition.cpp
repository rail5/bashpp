/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Method.h>
#include <IR/entities/Class.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ConstructorDefinition* node) {
	auto* current_class = dynamic_cast<bpp::IR::Class*>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor definition outside of class body");

	auto new_constructor = std::make_unique<bpp::IR::Method>();
	new_constructor->setName("__constructor");
	new_constructor->setScope(bpp::IR::VisibilityScope::PUBLIC);
	new_constructor->inherit(current_class);

	new_constructor->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	if (!new_constructor->addParameter(current_class->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to user-defined constructor");
	}

	if (const auto* parent_class = current_class->getParentClass()) {
		auto* parent_constructor = parent_class->getMethod_UNSAFE("__constructor");
		if (parent_constructor) {
			// FIXME(@rail5): Call parent constructor at the beginning of the child constructor.
		}
	}

	entity_stack.push(std::move(new_constructor));
}

template <>
void Listener::exit(ConstructorDefinition* node) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting ConstructorDefinition node");
	auto new_constructor = entity_stack.pop_as<bpp::IR::Method>();

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting ConstructorDefinition node");
	auto* current_class = entity_stack.top_as<bpp::IR::Class>();

	auto res = current_class->addMethod(std::move(new_constructor));
	if (!res) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor already defined in class '" + current_class->getName() + "'");
	}
}

} // namespace bpp::AST
