/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

/**
 * This file contains parser rules for all of:
 * 		1. Bash_case_statement
 * 		2. Bash_case_pattern
 * 		3. Bash_case_pattern_header
 */

#ifndef SRC_LISTENER_HANDLERS_BASH_CASE_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_BASH_CASE_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterBash_case_statement(BashppParser::Bash_case_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitBash_case_statement(BashppParser::Bash_case_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::enterBash_case_pattern(BashppParser::Bash_case_patternContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitBash_case_pattern(BashppParser::Bash_case_patternContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::enterBash_case_pattern_header(BashppParser::Bash_case_pattern_headerContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitBash_case_pattern_header(BashppParser::Bash_case_pattern_headerContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_BASH_CASE_STATEMENT_CPP_
