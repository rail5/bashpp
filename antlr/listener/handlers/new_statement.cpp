/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_NEW_STATEMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_NEW_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterNew_statement(BashppParser::New_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitNew_statement(BashppParser::New_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_NEW_STATEMENT_CPP_
