/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_comment
	skip_singlequote_string

	in_object_assignment = true;

	object_assignment_lvalue.clear();
	object_assignment_rvalue.clear();
	pre_objectassignment_code.clear();
	post_objectassignment_code.clear();
}

void BashppListener::exitObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_comment
	skip_singlequote_string

	in_object_assignment = false;

	std::string object_assignment_code = "eval " + object_assignment_lvalue + "=" + object_assignment_rvalue;

	// If we're not in a broader context, simply add the object assignment code to the program
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		current_program->add_code_to_previous_line(pre_objectassignment_code);
		current_program->add_code_to_next_line(post_objectassignment_code);
		current_program->add_code(object_assignment_code);
		return;
	}
}

#endif // ANTLR_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_
