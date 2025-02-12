/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_COMMENT_CPP_
#define SRC_LISTENER_HANDLERS_COMMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterComment(BashppParser::CommentContext *ctx) {
	skip_syntax_errors
	in_comment = true;
}

void BashppListener::exitComment(BashppParser::CommentContext *ctx) {
	skip_syntax_errors
	in_comment = false;

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity != nullptr) {
		// Add a newline
		current_code_entity->add_code("\n");
	}
}

#endif // SRC_LISTENER_HANDLERS_COMMENT_CPP_
