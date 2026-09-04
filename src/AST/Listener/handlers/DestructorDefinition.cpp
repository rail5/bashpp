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

	auto new_destructor = std::make_shared<bpp::IR::Method>();
	new_destructor->inherit(current_class);
	new_destructor->setName("__destructor");
	new_destructor->setScope(bpp::IR::VisibilityScope::PUBLIC);
	new_destructor->setIsVirtual(true);

	auto res = current_class->addMethod(std::move(new_destructor));
	if (!res) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor already defined in class '" + current_class->getName() + "'");
	}
	const auto& stored_destructor = res.value();
	stored_destructor->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	stored_destructor->addParameter(current_class->getThisPtr());

	entity_stack.push(stored_destructor);
}

template <>
void Listener::exit(DestructorDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method when exiting DestructorDefinition node");
	auto destructor = std::static_pointer_cast<bpp::IR::Method>(entity_stack.top());

	auto current_class = destructor->getContainingClass().lock();
	bpp_assert(current_class != nullptr, "Destructor's containing class is null when exiting DestructorDefinition node");
	auto parent_class = current_class->getParentClass();
	if (parent_class) {
		auto parent_destructor = parent_class->getMethod_UNSAFE("__destructor");
		if (parent_destructor) {
			// FIXME(@rail5): Call parent destructor at the end of the child destructor.

			parent_destructor->markReferencedBy(destructor);
		}
	}

	entity_stack.pop();
}

} // namespace bpp::AST
