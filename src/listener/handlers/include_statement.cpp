/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitInclude_statement(BashppParser::Include_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_INCLUDE_STATEMENT_CPP_
