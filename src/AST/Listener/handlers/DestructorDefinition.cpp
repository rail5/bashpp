/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Method.h>
#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(DestructorDefinition* node) {
	auto* current_class = dynamic_cast<bpp::IR::Class*>(entity_stack.top());
	if (!current_class) error(node, "Destructor definition outside of class body");

	auto new_destructor = std::make_unique<bpp::IR::Method>("__destructor");
	new_destructor->setScope(bpp::IR::VisibilityScope::PUBLIC);
	new_destructor->setIsVirtual(true);
	new_destructor->inherit(current_class);

	if (source_index) new_destructor->setDefinitionPosition(
		{getCurrentSourceFile(),
		node->getLine(), node->getColumn()}
	);

	// Optimization: We don't need to verify the `this` pointer of a destructor, since it's guaranteed to be valid.
	auto unverified_thisptr = std::make_unique<bpp::IR::UnverifiedThisPtr>(current_class);
	unverified_thisptr->inherit(new_destructor.get());
	if (!new_destructor->addParameter(std::move(unverified_thisptr))) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to user-defined destructor");
	}

	entity_stack.push(std::move(new_destructor));
}

template <>
void Listener::exit(DestructorDefinition* node) {
	bpp_assert(topmost_entity_is<bpp::IR::Method>(), "Topmost entity on stack is not a Method");
	auto destructor = entity_stack.pop_as<bpp::IR::Method>();

	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class");
	auto* current_class = entity_stack.top_as<bpp::IR::Class>();

	auto res = current_class->addMethod(std::move(destructor));
	if (!res) error(node, "Destructor already defined in class '" + current_class->getName() + "'");

	if (source_index) root_program->markContextEntity(
		{getCurrentSourceFile(),
		node->getLine(), node->getColumn(),
		node->getEndPosition().line, node->getEndPosition().column},
		*res
	);
}

} // namespace bpp::AST
