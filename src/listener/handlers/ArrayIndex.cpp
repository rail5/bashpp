/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterArrayIndex(std::shared_ptr<AST::ArrayIndex> node) {
	/**
	 * Array indices take the form:
	 * 	[...]
	 * Where the contents of the brackets are the array index
	 * And can be any valid sequence of statements
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}

	// Create a new code entity for the array index
	std::shared_ptr<bpp::bpp_string> array_index_entity = std::make_shared<bpp::bpp_string>();

	// Inherit the current code entity
	array_index_entity->set_containing_class(current_code_entity->get_containing_class());
	array_index_entity->inherit(current_code_entity);

	// Push the entity onto the stack
	entity_stack.push(array_index_entity);
}

void BashppListener::exitArrayIndex(std::shared_ptr<AST::ArrayIndex> node) {
	std::shared_ptr<bpp::bpp_string> array_index_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (array_index_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Array index context was not found in the entity stack");
	}

	entity_stack.pop();

	// If we're in an object reference, we have to call ->set_array_index

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());

	if (object_reference_entity != nullptr) {
		object_reference_entity->set_array_index("[" + array_index_entity->get_code() + "]");
		object_reference_entity->add_code_to_previous_line(array_index_entity->get_pre_code());
		object_reference_entity->add_code_to_next_line(array_index_entity->get_post_code());
		return;
	}

	// Otherwise, standard procedure
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}
	
	current_code_entity->add_code_to_previous_line(array_index_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(array_index_entity->get_post_code());
	current_code_entity->add_code("[" + array_index_entity->get_code() + "]");
}
