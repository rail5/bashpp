/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>

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
	if (current_code_entity->get_class(object_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, object_name, "Object name '" + object_name.getValue() + "' conflicts with a class name");
	}
	if (current_code_entity->get_object(object_name)) {
		throw bpp::ErrorHandling::SyntaxError(this, object_name, "Object '" + object_name.getValue() + "' already defined in this scope");
	}

	const auto object_class = current_code_entity->get_class(type_name);
	if (!object_class) {
		throw bpp::ErrorHandling::SyntaxError(this, type_name, "Class not found: '" + type_name.getValue() + "'");
	}

	object_class->add_reference_position({
		get_current_source_file(),
		type_name.getLine(),
		type_name.getCharPositionInLine()
	});

	auto object = std::make_shared<bpp::IR::Object>();
	object->inherit(current_code_entity);
	object->set_type(object_class);
	object->set_name(object_name);

	object->set_definition_position({
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
		datamember_declaration->set_type(object->get_type());
		datamember_declaration->set_name(object->get_name());
		if (object->has_initial_value()) datamember_declaration->set_initial_value(object->get_initial_value().value());
		datamember_declaration->set_definition_position(object->get_definition_position());
		return;
	}

	// Otherwise, add the object to the current code entity
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting ObjectInstantiation node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add_object(object);
}

} // namespace bpp::AST
