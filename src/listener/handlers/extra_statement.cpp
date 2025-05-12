/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_EXTRA_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_EXTRA_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterExtra_statement(BashppParser::Extra_statementContext *ctx) {
	skip_syntax_errors
	// If we're not in any broader context, simply add the statement to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code(ctx->getText());
		return;
	}
}

void BashppListener::exitExtra_statement(BashppParser::Extra_statementContext *ctx) {
	skip_syntax_errors
	}

#endif // SRC_LISTENER_HANDLERS_EXTRA_STATEMENT_CPP_
