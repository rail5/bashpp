/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_CLASS_BODY_STATEMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_CLASS_BODY_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterClass_body_statement(BashppParser::Class_body_statementContext *ctx) {
	skip_comment
	skip_singlequote_string

	// If we're not in any broader context,
	// And if this is merely a DELIM or WS token,
	// Simply add the code to the program
	std::shared_ptr<bpp::bpp_program> current_program = std::dynamic_pointer_cast<bpp::bpp_program>(entity_stack.top());
	if (current_program != nullptr) {
		// Either DELIM or WS should be set
		if (ctx->DELIM() != nullptr || ctx->WS() != nullptr) {
			program->add_code(ctx->getText());
			return;
		}
	} else {
		// If we're in a class body statement, we should be in a class
		std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
		if (current_class == nullptr) {
			throw syntax_error("Stray class body statement outside of class body", source_file, ctx->start->getLine(), ctx->start->getCharPositionInLine());
		}
	}
}

void BashppListener::exitClass_body_statement(BashppParser::Class_body_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_CLASS_BODY_STATEMENT_CPP_
