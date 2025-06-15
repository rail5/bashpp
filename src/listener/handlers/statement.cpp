/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterStatement(BashppParser::StatementContext *ctx) {
	skip_syntax_errors
}

void BashppListener::exitStatement(BashppParser::StatementContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		// Make sure we add terminal tokens to the program
		if (ctx->DELIM() != nullptr) {
			current_code_entity->add_code(ctx->DELIM()->getText());
			return;
		}
	}
}
