/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_STRING_CPP_
#define ANTLR_LISTENER_HANDLERS_STRING_CPP_

#include "../BashppListener.h"

void BashppListener::enterString(BashppParser::StringContext *ctx) {
	skip_comment
	skip_singlequote_string

	// If we're not in a broader context, simply add an open quote to the program
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		program->add_code("\"");
		return;
	}
}

void BashppListener::exitString(BashppParser::StringContext *ctx) {
	skip_comment
	skip_singlequote_string

	// If we're not in a broader context, simply add a close quote to the program
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		program->add_code("\"");
		return;
	}
}

#endif // ANTLR_LISTENER_HANDLERS_STRING_CPP_
