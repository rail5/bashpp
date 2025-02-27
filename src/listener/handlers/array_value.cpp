/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_ARRAY_VALUE_CPP_
#define SRC_LISTENER_HANDLERS_ARRAY_VALUE_CPP_

#include "../BashppListener.h"

void BashppListener::enterArray_value(BashppParser::Array_valueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Array values are rvalues in assignment operations which take the form
	 * (...)
	 * Where the contents of the parentheses are the array elements
	 * Empty arrays are written as ()
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw internal_error("Array value outside of code entity", ctx);
	}

	// Create a new code entity for the array value
	std::shared_ptr<bpp::bpp_string> arrayvalue_entity = std::make_shared<bpp::bpp_string>();
	arrayvalue_entity->set_containing_class(current_code_entity->get_containing_class());
	arrayvalue_entity->inherit(current_code_entity);

	// Push the entity onto the stack
	entity_stack.push(arrayvalue_entity);
}

void BashppListener::exitArray_value(BashppParser::Array_valueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> arrayvalue_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (arrayvalue_entity == nullptr) {
		throw internal_error("Array value context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	// Check if we're in a value_assignment context
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());

	if (value_assignment_entity != nullptr) {
		value_assignment_entity->set_array_assignment(true);
		value_assignment_entity->add_code_to_previous_line(arrayvalue_entity->get_pre_code());
		value_assignment_entity->add_code_to_next_line(arrayvalue_entity->get_post_code());
		value_assignment_entity->add_code("(" + arrayvalue_entity->get_code() + ")");
		return;
	}

	// If we're not in a value assignment context, that means we're assigning the array value to a primitive variable
	// The way to do that is just to add the code to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw internal_error("Array value outside of code entity", ctx);
	}

	current_code_entity->add_code_to_previous_line(arrayvalue_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(arrayvalue_entity->get_post_code());
	current_code_entity->add_code("(" + arrayvalue_entity->get_code() + ")");
}

#endif // SRC_LISTENER_HANDLERS_ARRAY_VALUE_CPP_
