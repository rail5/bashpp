/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_SINGLEQUOTE_STRING_CPP_
#define SRC_LISTENER_HANDLERS_SINGLEQUOTE_STRING_CPP_

#include "../BashppListener.h"

void BashppListener::enterSinglequote_string(BashppParser::Singlequote_stringContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
	in_singlequote_string = true;

	// If we're not in a broader context, simply add the entire string to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code(ctx->getText());
		return;
	}
}

void BashppListener::exitSinglequote_string(BashppParser::Singlequote_stringContext *ctx) {
	skip_comment
	skip_syntax_errors
	in_singlequote_string = false;
}

#endif // SRC_LISTENER_HANDLERS_SINGLEQUOTE_STRING_CPP_
