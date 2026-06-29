/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ClassDefinition* node) {
	// 1. Verify that we're at the top-level of the program
	if (!topmost_entity_is<bpp::IR::Program>()) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class definition must be at the top level of the program");
	}
	auto current_program = std::static_pointer_cast<bpp::IR::Program>(entity_stack.top());

	// 2. Verify that the class name is valid
	std::string class_name = node->CLASSNAME();
	if (!bpp::IR::is_valid_identifier(class_name)) {
		std::string msg = "Invalid class name: '" + class_name + "'";
		if (class_name.contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(class_name)) msg += " ('" + class_name + "' is a keyword)";
		throw bpp::ErrorHandling::SyntaxError(this, node, msg);
	}

	// 3. Verify that the class name is not already used in the program
	if (current_program->get_class(class_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class '" + class_name + "' already defined in program");
	}
	if (current_program->get_object(class_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class name '" + class_name + "' conflicts with an object in the program");
	}

	auto class_entity = std::make_shared<bpp::IR::Class>(class_name);
	class_entity->inherit(current_program);

	// Inherit from a parent class if specified
	if (node->PARENTCLASSNAME().has_value()) {
		auto parent_class_name = node->PARENTCLASSNAME().value().getValue();
		auto parent_class = current_program->get_class(parent_class_name);
		if (!parent_class) {
			throw bpp::ErrorHandling::SyntaxError(this, node, "Parent class '" + parent_class_name + "' not found");
		}
		class_entity->inherit(parent_class);

		parent_class->add_reference_position({
			get_current_source_file(),
			node->PARENTCLASSNAME().value().getLine(),
			node->PARENTCLASSNAME().value().getCharPositionInLine()
		});
	}

	class_entity->set_definition_position({
		get_current_source_file(),
		node->CLASSNAME().getLine(),
		node->CLASSNAME().getCharPositionInLine()
	});

	// Add placeholders for system methods:
	// __new, __delete, __copy, __constructor, __destructor
	// As well as a default 'toPrimitive' method
	// The contents of these methods will be filled in later
	auto new_method = std::make_shared<bpp::IR::Method>();
	new_method->set_name("__new");
	new_method->set_containing_class(class_entity);
	new_method->inherit(class_entity);
	auto requested_address_param = std::make_shared<bpp::IR::MethodParameter>(new_method);
	requested_address_param->set_name("__this");
	new_method->add_parameter(requested_address_param);

	auto delete_method = std::make_shared<bpp::IR::Method>();
	delete_method->set_name("__delete");
	delete_method->set_is_virtual(true);
	delete_method->set_containing_class(class_entity);
	delete_method->inherit(class_entity);
	delete_method->add_parameter(std::make_shared<bpp::IR::ThisPtr>(delete_method));

	auto copy_method = std::make_shared<bpp::IR::Method>();
	copy_method->set_name("__copy");
	copy_method->set_is_virtual(true);
	copy_method->set_containing_class(class_entity);
	copy_method->inherit(class_entity);
	copy_method->add_parameter(std::make_shared<bpp::IR::ThisPtr>(copy_method));

	auto constructor_method = std::make_shared<bpp::IR::Method>();
	constructor_method->set_name("__constructor");
	constructor_method->set_is_overridable(true);
	constructor_method->set_containing_class(class_entity);
	constructor_method->inherit(class_entity);
	constructor_method->add_parameter(std::make_shared<bpp::IR::ThisPtr>(constructor_method));

	auto destructor_method = std::make_shared<bpp::IR::Method>();
	destructor_method->set_name("__destructor");
	destructor_method->set_is_virtual(true);
	destructor_method->set_is_overridable(true);
	destructor_method->set_containing_class(class_entity);
	destructor_method->inherit(class_entity);
	destructor_method->add_parameter(std::make_shared<bpp::IR::ThisPtr>(destructor_method));

	auto toPrimitive_method = std::make_shared<bpp::IR::Method>();
	toPrimitive_method->set_name("toPrimitive");
	toPrimitive_method->set_containing_class(class_entity);
	toPrimitive_method->set_is_virtual(true);
	toPrimitive_method->set_is_overridable(true);
	toPrimitive_method->inherit(class_entity);
	toPrimitive_method->add_parameter(std::make_shared<bpp::IR::ThisPtr>(toPrimitive_method));
	toPrimitive_method->add("echo \"" + class_entity->get_name() + " Instance\"\n");

	class_entity->add_method(new_method);
	class_entity->add_method(delete_method);
	class_entity->add_method(copy_method);
	class_entity->add_method(constructor_method);
	class_entity->add_method(destructor_method);
	class_entity->add_method(toPrimitive_method);

	entity_stack.push(class_entity);
	current_program->add_class(class_entity); // Add the class to the program's list of known classes, so that it can be found by name later
	current_program->add(class_entity); // Add the class to the entity tree, so that it can be traversed later (e.g. for codegen)
	in_class = true;
}

template <>
void Listener::exit(ClassDefinition* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting ClassDefinition node");
	auto class_entity = std::static_pointer_cast<bpp::IR::Class>(entity_stack.top());
	entity_stack.pop();
	in_class = false;
}

} // namespace bpp::AST
