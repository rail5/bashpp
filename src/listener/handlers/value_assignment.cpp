/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_VALUE_ASSIGNMENT_CPP_
#define SRC_LISTENER_HANDLERS_VALUE_ASSIGNMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterValue_assignment(BashppParser::Value_assignmentContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
	
	std::shared_ptr<bpp::bpp_string> value_assignment_entity = std::make_shared<bpp::bpp_string>();
	value_assignment_entity->set_containing_class(entity_stack.top()->get_containing_class());
	entity_stack.push(value_assignment_entity);
}

void BashppListener::exitValue_assignment(BashppParser::Value_assignmentContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Value assignments will appear in the following contexts:
	 * 	1. Member declarations
	 * 	2. Object instantiations
	 * 	3. Pointer declarations
	 * 	4. Object assignments
	 */

	std::shared_ptr<bpp::bpp_string> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	entity_stack.pop();

	// Check if we're in a member declaration
	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (current_datamember != nullptr) {
		current_datamember->set_default_value(value_assignment_entity->get_code());
		current_datamember->set_pre_access_code(value_assignment_entity->get_pre_code());
		current_datamember->set_post_access_code(value_assignment_entity->get_post_code());
		return;
	}

	// Are we in an object_assignment rule?
	if (in_object_assignment) {
		pre_objectassignment_code += value_assignment_entity->get_pre_code();
		post_objectassignment_code += value_assignment_entity->get_post_code();
		object_assignment_rvalue = value_assignment_entity->get_code();
		return;
	}

	// Check if we're in an object instantiation or pointer declaration
	std::shared_ptr<bpp::bpp_object> current_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
	if (current_object != nullptr) {
		// Is it a pointer?
		if (current_object->is_pointer()) {
			current_object->set_address(value_assignment_entity->get_code());
		}
		// Else
	}

	// Check if we're in an object assignment

}

#endif // SRC_LISTENER_HANDLERS_VALUE_ASSIGNMENT_CPP_
