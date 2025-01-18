/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_CLASS_BODY_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_CLASS_BODY_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterClass_body_statement(BashppParser::Class_body_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	// If this is merely a DELIM or WS token,
	// Simply that DELIM or WS to wherever we are

	std::string delim_or_whitespace = "";
	bool just_delim_or_whitespace = false;
	if (ctx->DELIM() != nullptr || ctx->WS() != nullptr) {
		delim_or_whitespace = ctx->getText();
		just_delim_or_whitespace = true;
	}

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr && just_delim_or_whitespace) {
		current_code_entity->add_code(delim_or_whitespace);
		return;
	}

	if (just_delim_or_whitespace) {
		return;
	}
	
	if (current_code_entity == nullptr) {
		// If we're in a class body statement, we should be in a class
		std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
		if (current_class == nullptr) {
			throw_syntax_error_sym(ctx->start, "Stray class body statement outside of class body");
		}
	}
}

void BashppListener::exitClass_body_statement(BashppParser::Class_body_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_CLASS_BODY_STATEMENT_CPP_
