/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_DELETE_STATEMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_DELETE_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterDelete_statement(BashppParser::Delete_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitDelete_statement(BashppParser::Delete_statementContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_DELETE_STATEMENT_CPP_
