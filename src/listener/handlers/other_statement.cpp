/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterOther_statement(BashppParser::Other_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	// If we're in a class, throw an error
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (current_class != nullptr) {
		throw_syntax_error_sym(ctx->start, "Stray statement in class definition");
		return;
	}

	// If we're not in any broader context, simply add the statement to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code(ctx->getText());
		return;
	}
}

void BashppListener::exitOther_statement(BashppParser::Other_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_
