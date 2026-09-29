/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Method.h>
#include <IR/entities/expressions/ObjectInstantiation.h>
#include <IR/entities/expressions/ObjectReference.h>
#include <IR/entities/expressions/ObjectAssignment.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(ObjectInstantiation* node) {
	// Note: All object instantiations must be either
	// a) directly inside a code entity, or
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

	const auto* current_code_entity = latest_code_entity();
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

	const auto* object_class = current_code_entity->getClass(type_name);
	if (!object_class) {
		throw bpp::ErrorHandling::SyntaxError(this, type_name, "Class not found: '" + type_name.getValue() + "'");
	}

	auto object = std::make_unique<bpp::IR::Object>();
	object->inherit(current_code_entity);
	object->setType(object_class);
	object->setIsPointer(node->isPointer());
	object->setName(object_name);

	object->setDefinitionPosition({
		get_current_source_file(),
		object_name.getLine(),
		object_name.getCharPositionInLine()
	});

	entity_stack.push(std::move(object));
}

template <>
void Listener::exit(ObjectInstantiation* node) {
	bpp_assert(topmost_entity_is<bpp::IR::Object>(), "Topmost entity on stack is not an Object when exiting ObjectInstantiation node");
	auto object = entity_stack.pop_as<bpp::IR::Object>();

	if (auto* datamember_declaration = dynamic_cast<bpp::IR::DataMember*>(entity_stack.top())) {
		// This object instantiation is part of a class's data member declaration
		// The data for this object should be moved to the data member, and the object should be discarded
		datamember_declaration->setType(object->getType());
		datamember_declaration->setIsPointer(object->isPointer());
		datamember_declaration->setName(object->getName());
		if (object->hasInitialValue()) datamember_declaration->setInitialValue(object->releaseInitialValue());
		datamember_declaration->setDefinitionPosition(object->getDefinitionPosition());
		return;
	}

	// Otherwise, add the object to the current code entity
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting ObjectInstantiation node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	if (!object->isPointer()) {
		const auto* object_class = object->getType();
		bpp_assert(object_class != nullptr, "Object has no type when exiting ObjectInstantiation node");
		
		auto instantiation = std::make_unique<bpp::IR::ObjectInstantiation>();
		instantiation->inherit(current_code_entity);
		instantiation->setType(object_class);
		instantiation->setStackLikeObject(object.get());

		current_code_entity->add(std::move(instantiation));
	} else {
		auto object_reference = std::make_unique<bpp::IR::ObjectReference>();
		object_reference->inherit(current_code_entity);
		object_reference->setReferenceChain(bpp::IR::ObjectReference::ReferenceChain(object.get()));

		auto pointer_assignment = std::make_unique<bpp::IR::ObjectAssignment>();
		pointer_assignment->inherit(current_code_entity);
		pointer_assignment->setLHS(std::move(object_reference));

		if (object->hasInitialValue()) {
			bpp_assert(dynamic_cast<const bpp::IR::ValueAssignment*>(object->getInitialValue().value()), "Object initial value is not a ValueAssignment when exiting ObjectInstantiation node");
			auto va = std::unique_ptr<bpp::IR::ValueAssignment>(static_cast<bpp::IR::ValueAssignment*>(object->releaseInitialValue().release()));
			pointer_assignment->setRHS(std::move(va));
		} else {
			auto va = std::make_unique<bpp::IR::ValueAssignment>();
			va->inherit(current_code_entity);
			va->add("0"); // Default-initialize pointers to null
			pointer_assignment->setRHS(std::move(va));
		}

		current_code_entity->add(std::move(pointer_assignment));
	}

	if (!current_code_entity->addObject(std::move(object))) {
		const auto* named_code_entity = dynamic_cast<bpp::IR::NamedEntity*>(current_code_entity);
		std::string error_message = "Failed to add object '" + node->IDENTIFIER().getValue() + "' to code entity";
		if (named_code_entity) error_message += " '" + named_code_entity->getName() + "'";
		throw bpp::ErrorHandling::InternalError(error_message);
	}
}

} // namespace bpp::AST
