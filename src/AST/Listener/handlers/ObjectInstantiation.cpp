/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Method.h>
#include <IR/entities/expressions/ObjectReference.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ObjectInstantiation* node) {
	// Note: All object instantiations must be
	// a) directly inside a code entity
	// b) part of a data member declaration
	if (!topmost_entity_is<bpp::IR::CodeEntity>() && !topmost_entity_is<bpp::IR::DataMember>()) {
		// Special case to provide more useful information to the user:
		// If the topmost entity is a Class, the user might have simply forgotten the visibility modifier
		// which would've made this statement a data member declaration.
		if (topmost_entity_is<bpp::IR::Class>()) {
			throw bpp::ErrorHandling::SyntaxError(this, node, "Stray object instantiation inside class body.\n"
				"Did you mean to declare a data member?\n"
				"Start with a visibility modifier (@public, @private, @protected) to declare a data member");
		}
		throw bpp::ErrorHandling::SyntaxError(this, node, "Object instantiation outside of a code entity");
	}

	const auto& type_name = node->TYPE();
	const auto& object_name = node->IDENTIFIER();

	const auto current_code_entity = latest_code_entity();
	bpp_assert(current_code_entity != nullptr, "No code entity found on stack when entering ObjectInstantiation node");

	// Name validation
	// 1. Valid identifier?
	if (!bpp::IR::is_valid_identifier(object_name)) {
		std::string msg = "Invalid object name: '" + object_name.getValue() + "'";
		if (object_name.getValue().contains("__")) msg += " (Bash++ identifiers cannot contain double underscores)";
		if (bpp::IR::is_protected_keyword(object_name)) msg += " ('" + object_name.getValue() + "' is a keyword)";
		throw bpp::ErrorHandling::SyntaxError(this, object_name, msg);
	}
	// 2. Name already in use?
	if (current_code_entity->getClass(object_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, object_name, "Object name '" + object_name.getValue() + "' conflicts with a class name");
	}
	if (current_code_entity->getObject(object_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, object_name, "Object '" + object_name.getValue() + "' already defined in this scope");
	}

	const auto object_class = current_code_entity->getClass(type_name);
	if (!object_class) {
		throw bpp::ErrorHandling::SyntaxError(this, type_name, "Class not found: '" + type_name.getValue() + "'");
	}

	object_class->addReferencePosition({
		get_current_source_file(),
		type_name.getLine(),
		type_name.getCharPositionInLine()
	});

	auto object = std::make_shared<bpp::IR::Object>();
	object->inherit(current_code_entity);
	object->setType(object_class);
	object->setIsPointer(node->isPointer());
	object->setName(object_name);

	object->setDefinitionPosition({
		get_current_source_file(),
		object_name.getLine(),
		object_name.getCharPositionInLine()
	});

	entity_stack.push(object);
}

template <>
void Listener::exit(ObjectInstantiation* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Object>(), "Topmost entity on stack is not an Object when exiting ObjectInstantiation node");
	auto object = std::static_pointer_cast<bpp::IR::Object>(entity_stack.top());
	entity_stack.pop();

	if (auto datamember_declaration = std::dynamic_pointer_cast<bpp::IR::DataMember>(entity_stack.top())) {
		// This object instantiation is part of a class's data member declaration
		// The data for this object should be moved to the data member, and the object should be discarded
		datamember_declaration->setType(object->getType());
		datamember_declaration->setIsPointer(object->isPointer());
		datamember_declaration->setName(object->getName());
		if (object->hasInitialValue()) datamember_declaration->setInitialValue(object->getInitialValue().value());
		datamember_declaration->setDefinitionPosition(object->getDefinitionPosition());
		return;
	}

	// Otherwise, add the object to the current code entity
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting ObjectInstantiation node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	if (!current_code_entity->addObject(object)) {
		const auto named_code_entity = std::dynamic_pointer_cast<bpp::IR::NamedEntity>(current_code_entity);
		std::string error_message = "Failed to add object '" + object->getName() + "' to code entity";
		if (named_code_entity) error_message += " '" + named_code_entity->getName() + "'";
		throw bpp::ErrorHandling::InternalError(error_message);
	}

	if (!object->isPointer()) {
		// Add a call to the class's __new method to instantiate the object
		const auto object_class = object->getType().lock();
		bpp_assert(object_class != nullptr, "Object has no type when exiting ObjectInstantiation node");
		auto new_method = object_class->getMethod_UNSAFE("__new");
		bpp_assert(new_method != nullptr, "Class '" + object_class->getName() + "' does not have a '__new' method");

		auto method_call = std::make_shared<bpp::IR::ObjectReference>();
		method_call->inherit(current_code_entity);
		bpp::IR::ObjectReference::ReferenceChain chain(object);
		chain.setMethod(new_method);
		method_call->setReferenceChain(std::move(chain));
		method_call->setLvalue(true);
		current_code_entity->add(method_call);
		current_code_entity->add(" >/dev/null\n"); // Discard the output of the __new method, since it will echo the address of the new object

		// Mark the class's "__new" method as used
		new_method->markReferencedBy(method_call);
	} else {
		current_code_entity->add(object->getAddress());
		if (object->hasInitialValue()) {
			current_code_entity->add(object->getInitialValue().value());
		} else {
			current_code_entity->add("=0"); // Default-initialize pointers to null
		}
	}
}

} // namespace bpp::AST
