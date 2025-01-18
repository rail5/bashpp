/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterOther_statement(BashppParser::Other_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	if (in_string) {
		current_string_contents += ctx->getText();
		return;
	}

	if (in_value_assignment) {
		value_assignment += ctx->getText();
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

#endif // ANTLR_LISTENER_HANDLERS_OTHER_STATEMENT_CPP_
