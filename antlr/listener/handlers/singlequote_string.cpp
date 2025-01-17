/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_SINGLEQUOTE_STRING_CPP_
#define ANTLR_LISTENER_HANDLERS_SINGLEQUOTE_STRING_CPP_

#include "../BashppListener.h"

void BashppListener::enterSinglequote_string(BashppParser::Singlequote_stringContext *ctx) {
	skip_comment
	skip_singlequote_string
	in_singlequote_string = true;

	if (in_value_assignment) {
		value_assignment += ctx->getText();
	}

	// If we're not in a broader context, simply add the entire string to the program
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		program->add_code(ctx->getText());
		return;
	}
}

void BashppListener::exitSinglequote_string(BashppParser::Singlequote_stringContext *ctx) {
	skip_comment
	in_singlequote_string = false;
}

#endif // ANTLR_LISTENER_HANDLERS_SINGLEQUOTE_STRING_CPP_
