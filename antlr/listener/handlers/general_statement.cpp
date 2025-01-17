/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_GENERAL_STATEMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_GENERAL_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterGeneral_statement(BashppParser::General_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
	
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		if (ctx->DELIM() != nullptr || ctx->WS() != nullptr) {
			program->add_code(ctx->getText());
			return;
		}
	}
}

void BashppListener::exitGeneral_statement(BashppParser::General_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_GENERAL_STATEMENT_CPP_
