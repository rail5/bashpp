/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/identifiers.h>

#include <IR/entities/Class.h>
#include <IR/entities/SystemMethod.h>
#include <IR/entities/MethodParameter.h>
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
	auto* current_program = entity_stack.top_as<bpp::IR::Program>();

	// 2. Verify that the class name is valid
	std::string class_name = node->CLASSNAME();
	if (!bpp::IR::is_valid_identifier(class_name)) {
		std::string msg = "Invalid class name: '" + class_name + "'";
		if (class_name.contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(class_name)) msg += " ('" + class_name + "' is a keyword)";
		throw bpp::ErrorHandling::SyntaxError(this, node, msg);
	}

	// 3. Verify that the class name is not already used in the program
	if (current_program->getClass(class_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class '" + class_name + "' already defined in program");
	}
	if (current_program->getObject(class_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Class name '" + class_name + "' conflicts with an object in the program");
	}

	auto class_entity = std::make_unique<bpp::IR::Class>(class_name);
	current_program->preregisterClass(class_entity.get());
	class_entity->inherit(current_program);

	// Inherit from a parent class if specified
	if (node->PARENTCLASSNAME().has_value()) {
		auto parent_class_name = node->PARENTCLASSNAME().value().getValue();
		auto* parent_class = current_program->getClass(parent_class_name);
		if (!parent_class) {
			current_program->unPreregisterClass();
			throw bpp::ErrorHandling::SyntaxError(this, node, "Parent class '" + parent_class_name + "' not found");
		}
		class_entity->inherit(parent_class);
	}

	class_entity->setDefinitionPosition({
		get_current_source_file(),
		node->CLASSNAME().getLine(),
		node->CLASSNAME().getCharPositionInLine()
	});

	// Add placeholders for system methods:
	// __new, __delete, __copy, __constructor, __destructor
	// As well as a default 'toPrimitive' method
	// The contents of these methods will be filled in later
	auto new_method = std::make_unique<bpp::IR::Builtins::SystemMethod>(bpp::IR::Builtins::SystemMethod::Type::NEW);
	new_method->inherit(class_entity.get());
	auto requested_address_param = std::make_unique<bpp::IR::RequestedAddressParam>(class_entity.get());
	requested_address_param->inherit(new_method.get());
	if (!new_method->addParameter(std::move(requested_address_param))) {
		throw bpp::ErrorHandling::InternalError("Failed to add requested address parameter to __new method");
	}
	new_method->setScope(bpp::IR::VisibilityScope::PUBLIC);

	auto delete_method = std::make_unique<bpp::IR::Builtins::SystemMethod>(bpp::IR::Builtins::SystemMethod::Type::DELETE);
	delete_method->setIsVirtual(true);
	delete_method->inherit(class_entity.get());
	if (!delete_method->addParameter(class_entity->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to __delete method");
	}
	delete_method->setScope(bpp::IR::VisibilityScope::PUBLIC);

	auto copy_method = std::make_unique<bpp::IR::Builtins::SystemMethod>(bpp::IR::Builtins::SystemMethod::Type::COPY);
	copy_method->setIsVirtual(true);
	copy_method->inherit(class_entity.get());
	if (!copy_method->addParameter(class_entity->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to __copy method");
	}
	auto copy_from_param = std::make_unique<bpp::IR::CopyFromParam>(class_entity.get());
	copy_from_param->inherit(copy_method.get());
	if (!copy_method->addParameter(std::move(copy_from_param))) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'copy from' parameter to __copy method");
	}
	copy_method->setScope(bpp::IR::VisibilityScope::PUBLIC);


	auto constructor_method = std::make_unique<bpp::IR::Method>();
	constructor_method->setName("__constructor");
	constructor_method->setIsOverridable(true);
	constructor_method->inherit(class_entity.get());
	if (!constructor_method->addParameter(class_entity->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to auto-generated constructor method");
	}
	constructor_method->setScope(bpp::IR::VisibilityScope::PUBLIC);

	auto destructor_method = std::make_unique<bpp::IR::Method>();
	destructor_method->setName("__destructor");
	destructor_method->setIsVirtual(true);
	destructor_method->setIsOverridable(true);
	destructor_method->inherit(class_entity.get());
	if (!destructor_method->addParameter(class_entity->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to auto-generated destructor method");
	}
	destructor_method->setScope(bpp::IR::VisibilityScope::PUBLIC);

	auto toPrimitive_method = std::make_unique<bpp::IR::Method>();
	toPrimitive_method->setName("toPrimitive");
	toPrimitive_method->setIsVirtual(true);
	toPrimitive_method->setIsOverridable(true);
	toPrimitive_method->inherit(class_entity.get());
	if (!toPrimitive_method->addParameter(class_entity->getThisPtr())) {
		throw bpp::ErrorHandling::InternalError("Failed to add 'this' parameter to auto-generated toPrimitive method");
	}
	toPrimitive_method->setScope(bpp::IR::VisibilityScope::PUBLIC);
	toPrimitive_method->add("echo \"" + class_entity->getName() + " Instance\"\n");

	auto add_system_method = [&](std::unique_ptr<bpp::IR::Method> method) {
		if (!class_entity->addMethod(std::move(method))) {
			throw bpp::ErrorHandling::InternalError("Failed to add system method to class '" + class_entity->getName() + "'");
		}
	};

	add_system_method(std::move(new_method));
	add_system_method(std::move(delete_method));
	add_system_method(std::move(copy_method));
	add_system_method(std::move(constructor_method));
	add_system_method(std::move(destructor_method));
	add_system_method(std::move(toPrimitive_method));

	entity_stack.push(std::move(class_entity));
}

template <>
void Listener::exit(ClassDefinition* node) {
	bpp_assert(topmost_entity_is<bpp::IR::Class>(), "Topmost entity on stack is not a Class when exiting ClassDefinition node");
	auto class_entity = entity_stack.pop_as<bpp::IR::Class>();

	bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program when exiting ClassDefinition node");
	auto* current_program = entity_stack.top_as<bpp::IR::Program>();

	current_program->unPreregisterClass(); // Remove the preregistered class pointer, since the class has now been fully defined

	auto res = current_program->addClass(std::move(class_entity));
	if (!res) {
		std::string error_message = "Class name conflicts with existing ";
		switch (res.error()) {
			case bpp::IR::NameConflictError::EXISTING_CLASS: error_message += "class"; break;
			case bpp::IR::NameConflictError::EXISTING_OBJECT: error_message += "object"; break;
			default: error_message += "entity"; break;
		}
		error_message += ": " + node->CLASSNAME().getValue();
		throw bpp::ErrorHandling::SyntaxError(this, node, error_message);
	}
}

} // namespace bpp::AST
