/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Class.h>
#include <IR/entities/Program.h>

namespace bpp::AST {

template <>
void Listener::enter(ClassDefinition* node) {
	// 1. Verify that we're at the top-level of the program
	if (!topmost_entity_is<bpp::IR::Program>()) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class definition must be at the top level of the program");
	}

	// 2. Verify that the class name is valid
	std::string class_name = node->CLASSNAME();
	if (!bpp::IR::is_valid_identifier(class_name)) {
		std::string msg = "Invalid class name: '" + class_name + "'";
		if (class_name.contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(class_name)) msg += " ('" + class_name + "' is a keyword)";
		throw bpp::ErrorHandling::SyntaxError(this, node, msg);
	}

	// 3. Verify that the class name is not already used in the program
	if (program->get_class(class_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class '" + class_name + "' already defined in program");
	}
	if (program->get_object(class_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class name '" + class_name + "' conflicts with an object in the program");
	}

	auto class_entity = std::make_shared<bpp::IR::Class>(class_name);
	class_entity->inherit(program);

	// Inherit from a parent class if specified
	if (node->PARENTCLASSNAME().has_value()) {
		auto parent_class_name = node->PARENTCLASSNAME().value().getValue();
		auto parent_class = program->get_class(parent_class_name);
		if (!parent_class) {
			throw bpp::ErrorHandling::SyntaxError(this, node, "Parent class '" + parent_class_name + "' not found");
		}
		class_entity->inherit(parent_class);
	}


	entity_stack.push(class_entity);
	in_class = true;
}

template <>
void Listener::exit(ClassDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting ClassDefinition node");
	auto class_entity = std::static_pointer_cast<bpp::IR::Class>(entity_stack.top());
	entity_stack.pop();
	program->add(class_entity);
	in_class = false;
}

} // namespace bpp::AST
