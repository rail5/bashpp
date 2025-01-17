/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_STRING_CPP_
#define ANTLR_LISTENER_HANDLERS_STRING_CPP_

#include "../BashppListener.h"

void BashppListener::enterString(BashppParser::StringContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	in_string = true;

	current_string_contents.clear();
	pre_string_code.clear();
	post_string_code.clear();

	if (in_value_assignment) {
		// Add an open-quote to the value assignment
		value_assignment += "\"";
		return;
	}

	// If we're not in a broader context, simply add an open quote to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code("\"");
		return;
	}
}

void BashppListener::exitString(BashppParser::StringContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	in_string = false;

	if (in_value_assignment) {
		pre_valueassignment_code += pre_string_code;
		post_valueassignment_code += post_string_code;
		value_assignment += current_string_contents;
		value_assignment += "\""; // Add a close quote to the value assignment
		return;
	}

	// If we're not in a broader context, simply add the current string contents + a close quote to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(pre_string_code);
		current_code_entity->add_code_to_next_line(post_string_code);
		current_code_entity->add_code(current_string_contents + "\"");
		return;
	}
}

#endif // ANTLR_LISTENER_HANDLERS_STRING_CPP_
