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
	if (!in_value_assignment) {
		return;
	}

	// One of either IDENTIFIER, NUMBER, or BASH_VAR will be set
	if (ctx->IDENTIFIER() != nullptr) {
		value_assignment += ctx->IDENTIFIER()->getText();
	} else if (ctx->NUMBER() != nullptr) {
		value_assignment += ctx->NUMBER()->getText();
	} else if (ctx->BASH_VAR() != nullptr) {
		value_assignment += ctx->BASH_VAR()->getText();
	}
}

void BashppListener::exitRaw_rvalue(BashppParser::Raw_rvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_RAW_RVALUE_CPP_
