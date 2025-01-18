/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_
#define SRC_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		current_code_entity = program;
	}

	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::make_shared<bpp::bpp_object_assignment>();
	object_assignment->set_containing_class(entity_stack.top()->get_containing_class());
	object_assignment->inherit(current_code_entity);
	entity_stack.push(object_assignment);
}

void BashppListener::exitObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
	entity_stack.pop();

	if (object_assignment == nullptr) {
		throw internal_error("Object assignment context was not found in the entity stack");
	}

	std::string object_assignment_lvalue = object_assignment->get_lvalue();
	std::string object_assignment_rvalue = object_assignment->get_rvalue();
	std::string pre_objectassignment_code = object_assignment->get_pre_code();
	std::string post_objectassignment_code = object_assignment->get_post_code();

	pre_objectassignment_code += "____assignmentRVal=" + object_assignment_rvalue + "\n";
	post_objectassignment_code += "unset ____assignmentRVal\n";

	std::string object_assignment_code = "eval " + object_assignment_lvalue + "=\\$____assignmentRVal\n";

	// If we're not in a broader context, simply add the object assignment code to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(pre_objectassignment_code);
		current_code_entity->add_code_to_next_line(post_objectassignment_code);
		current_code_entity->add_code(object_assignment_code);
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_
