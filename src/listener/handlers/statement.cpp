/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterStatement(BashppParser::StatementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitStatement(BashppParser::StatementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		// Make sure we add terminal tokens to the program
		if (ctx->DELIM() != nullptr) {
			program->add_code(ctx->DELIM()->getText());
			return;
		}
	}
}

#endif // SRC_LISTENER_HANDLERS_STATEMENT_CPP_
