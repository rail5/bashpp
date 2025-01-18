/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_RAW_RVALUE_CPP_
#define SRC_LISTENER_HANDLERS_RAW_RVALUE_CPP_

#include "../BashppListener.h"

void BashppListener::enterRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code(ctx->getText());
		return;
	}
}

void BashppListener::exitRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_RAW_RVALUE_CPP_
