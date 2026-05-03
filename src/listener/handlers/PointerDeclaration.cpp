/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <listener/BashppListener.h>

void BashppListener::enterPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node) {
	auto object_type = node->TYPE();
	auto object_name = node->IDENTIFIER();

	// Verify that we're in a place where an object *can* be instantiated
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (current_class != nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Stray pointer declaration inside class body.\nDid you mean to declare a data member?\nIf so, start by declaring the data member with a visibility keyword (@public, @private, @protected)");
	}

	// Actually get the containing class
	current_class = entity_stack.top()->get_containing_class().lock();

	const std::string& object_type_text = object_type.getValue();
	const std::string& object_name_text = object_name.getValue();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = latest_code_entity();

	// Are we in a data member declaration?
	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());

	std::shared_ptr<bpp::bpp_class> object_class = current_code_entity->get_class(object_type_text);

	// Verify that the object's class exists
	if (object_class == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, object_type, "Class not found: " + object_type_text);
	}

	std::shared_ptr<bpp::bpp_object> new_object = std::make_shared<bpp::bpp_object>();
	new_object->set_name(object_name_text);
	new_object->set_pointer(true);
	new_object->set_containing_class(current_class);
	entity_stack.push(new_object);

	new_object->set_definition_position(
		source_file,
		object_name.getLine(),
		object_name.getCharPositionInLine()
	);

	new_object->set_class(object_class);

	object_class->add_reference(
		source_file,
		object_type.getLine(),
		object_type.getCharPositionInLine()
	);

	new_object->set_address("bpp____ptr__" + std::to_string(program->get_object_counter()) + "__" + new_object->get_class()->get_name() + "__" + new_object->get_name());
	program->increment_object_counter();

	// Verify the object's name is valid
	if (!bpp::is_valid_identifier(new_object->get_name())) {
		entity_stack.pop();
		// If, specifically, it contains a double underscore, we can provide a more specific error message
		if (new_object->get_name().contains("__")) {
			throw bpp::ErrorHandling::SyntaxError(this, object_name, "Invalid object name: " + new_object->get_name() + "\nBash++ identifiers cannot contain double underscores");
		} else {
			throw bpp::ErrorHandling::SyntaxError(this, object_name, "Invalid object name: " + new_object->get_name());
		}
	}

	if (current_code_entity->get_class(new_object->get_name()) != nullptr) {
		entity_stack.pop();
		throw bpp::ErrorHandling::SyntaxError(this, object_name, "Class already exists: " + new_object->get_name());
	}
	if (current_code_entity->get_object(new_object->get_name()) != nullptr) {
		entity_stack.pop();
		throw bpp::ErrorHandling::SyntaxError(this, object_name, "Object already exists: " + new_object->get_name());
	}
}

void BashppListener::exitPointerDeclaration(std::shared_ptr<AST::PointerDeclaration> node) {
	bpp_assert(topmost_entity_is<bpp::bpp_object>(), "Pointer declaration context was not found in the entity stack");
	auto new_object = std::static_pointer_cast<bpp::bpp_object>(entity_stack.top());

	entity_stack.pop();

	// If there is no assignment value set, set it to nullptr
	if (new_object->get_assignment_value().empty()) {
		new_object->set_nullptr();
	}

	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (current_datamember != nullptr) {
		// We're midway through a class member declaration
		// The data for this object should be moved to the datamember
		current_datamember->set_class(new_object->get_class());
		current_datamember->set_name(new_object->get_name());
		current_datamember->set_pointer(true);
		current_datamember->set_pre_access_code(new_object->get_pre_access_code());
		current_datamember->set_post_access_code(new_object->get_post_access_code());
		current_datamember->set_default_value(new_object->get_assignment_value());
		current_datamember->set_definition_position(
			source_file,
			new_object->get_initial_definition().line,
			new_object->get_initial_definition().column
		);
		return;
	}

	// Add the object to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Pointer declaration outside of code entity");
	}

	current_code_entity->add_code_to_previous_line(new_object->get_pre_access_code());
	current_code_entity->add_code_to_next_line(new_object->get_post_access_code());
	current_code_entity->add_object(new_object, should_localize_object_instantiation());
}
