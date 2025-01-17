/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_VALUE_ASSIGNMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_VALUE_ASSIGNMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterValue_assignment(BashppParser::Value_assignmentContext *ctx) {
	skip_comment
	skip_singlequote_string
	in_value_assignment = true;
	pre_valueassignment_code.clear();
	post_valueassignment_code.clear();
	value_assignment.clear();
}

void BashppListener::exitValue_assignment(BashppParser::Value_assignmentContext *ctx) {
	skip_comment
	skip_singlequote_string
	in_value_assignment = false;

	/**
	 * Value assignments will appear in the following contexts:
	 * 	1. Member declarations
	 * 	2. Object instantiations
	 * 	3. Pointer declarations
	 * 	4. Object assignments
	 */

	// Check if we're in a member declaration
	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (current_datamember != nullptr) {
		current_datamember->set_default_value(value_assignment);
		current_datamember->set_pre_access_code(pre_valueassignment_code);
		current_datamember->set_post_access_code(post_valueassignment_code);
		return;
	}

	// Check if we're in an object instantiation or pointer declaration
	std::shared_ptr<bpp::bpp_object> current_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
	if (current_object != nullptr) {
		// Is it a pointer?
		if (current_object->is_pointer()) {
			current_object->set_address(value_assignment);
		}
		// Else
	}

	// Check if we're in an object assignment

}

#endif // ANTLR_LISTENER_HANDLERS_VALUE_ASSIGNMENT_CPP_