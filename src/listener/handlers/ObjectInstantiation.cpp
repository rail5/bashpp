/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node) {
	skip_syntax_errors
	/**
	 * The object type will be stored in one of either IDENTIFIER_LVALUE or IDENTIFIER(0)
	 * If IDENTIFIER_LVALUE, then the object name will be in IDENTIFIER(0)
	 * If IDENTIFIER(0), then the object name will be in IDENTIFIER(1)
	 */

	// Verify that we're in a place where an object *can* be instantiated
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	auto object_type = node->TYPE();
	auto object_name = node->IDENTIFIER();

	if (current_class != nullptr) {
		syntax_error(node, "Stray object instantiation inside class body.\nDid you mean to declare a data member?\nIf so, start by declaring the data member with a visibility keyword (@public, @private, @protected)");
	}

	std::string object_type_text = object_type.getValue();
	std::string object_name_text = object_name.getValue();
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = latest_code_entity();

	std::shared_ptr<bpp::bpp_object> new_object = std::make_shared<bpp::bpp_object>(object_name_text);
	entity_stack.push(new_object);

	new_object->set_definition_position(
		source_file,
		object_name.getLine(),
		object_name.getCharPositionInLine()
	);

	std::shared_ptr<bpp::bpp_class> object_class = current_code_entity->get_class(object_type_text);

	// Verify that the object's class exists
	if (object_class == nullptr) {
		entity_stack.pop();
		syntax_error(object_type, "Class not found: " + object_type_text);
	}

	new_object->set_class(object_class);

	object_class->add_reference(
		source_file,
		object_type.getLine(),
		object_type.getCharPositionInLine()
	);

	new_object->set_containing_class(current_code_entity->get_containing_class());

	// Note:
	// Here, we're incrementing an internal object counter and determining the location of the object in memory AT COMPILE-TIME
	// Should this be determined at run-time instead?
	new_object->set_address("bpp__" + std::to_string(program->get_object_counter()) + "__" + object_class->get_name() + "__" + new_object->get_name());
	program->increment_object_counter();

	// Verify the object's name is valid
	if (!bpp::is_valid_identifier(new_object->get_name())) {
		entity_stack.pop();
		// If, specifically, it contains a double underscore, we can provide a more specific error message
		if (new_object->get_name().find("__") != std::string::npos) {
			syntax_error(object_name, "Invalid object name: " + new_object->get_name() + "\nBash++ identifiers cannot contain double underscores");
		} else {
			syntax_error(object_name, "Invalid object name: " + new_object->get_name());
		}
	}

	// Verify that the object name is not already in use
	if (current_code_entity->get_class(new_object->get_name()) != nullptr) {
		entity_stack.pop();
		syntax_error(object_name, "Class already exists: " + new_object->get_name());
	}
	if (current_code_entity->get_object(new_object->get_name()) != nullptr) {
		entity_stack.pop();
		syntax_error(object_name, "Object already exists: " + new_object->get_name());
	}
}

void BashppListener::exitObjectInstantiation(std::shared_ptr<AST::ObjectInstantiation> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_object> new_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
	if (new_object == nullptr) {
		throw internal_error("entity_stack top is not a bpp_object");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (current_datamember != nullptr) {
		// We're midway through a class member declaration
		// The data for this object should be moved to the datamember
		current_datamember->set_class(new_object->get_class());
		current_datamember->set_name(new_object->get_name());
		current_datamember->set_definition_position(
			source_file,
			new_object->get_initial_definition().line,
			new_object->get_initial_definition().column
		);
		return;
	}

	// Add the object to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_object(new_object);
		return;
	}
}
