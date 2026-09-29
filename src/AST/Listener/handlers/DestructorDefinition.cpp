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
void Listener::enter(DestructorDefinition* node) {
	auto* current_class = dynamic_cast<bpp::IR::Class*>(entity_stack.top());
	if (!current_class) throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor definition outside of class body");

	auto new_destructor = std::make_unique<bpp::IR::Method>();
	new_destructor->setName("__destructor");
	new_destructor->setScope(bpp::IR::VisibilityScope::PUBLIC);
	new_destructor->setIsVirtual(true);
	new_destructor->inherit(current_class);

	new_destructor->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	if (!new_destructor->addParameter(current_class->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to user-defined destructor");
	}

	entity_stack.push(std::move(new_destructor));
}

template <>
void Listener::exit(DestructorDefinition* node) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting DestructorDefinition node");
	auto destructor = entity_stack.pop_as<bpp::IR::Method>();

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting DestructorDefinition node");
	auto* current_class = entity_stack.top_as<bpp::IR::Class>();

	const auto* parent_class = current_class->getParentClass();
	if (parent_class) {
		auto* parent_destructor = parent_class->getMethod_UNSAFE("__destructor");
		if (parent_destructor) {
			// FIXME(@rail5): Call parent destructor at the end of the child destructor.
		}
	}

	auto res = current_class->addMethod(std::move(destructor));
	if (!res) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor already defined in class '" + current_class->getName() + "'");
	}
}

} // namespace bpp::AST
