/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterOther_statement(BashppParser::Other_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitOther_statement(BashppParser::Other_statementContext *ctx) {
	skip_comment
	skip_singlequote_string

	// If we're not in any broader context, simply add the statement to the program
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		program->add_code(ctx->getText());
		return;
	}
}

#endif // ANTLR_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_
