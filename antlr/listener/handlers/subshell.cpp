/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_SUBSHELL_CPP_
#define ANTLR_LISTENER_HANDLERS_SUBSHELL_CPP_

#include "../BashppListener.h"

void BashppListener::enterSubshell(BashppParser::SubshellContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitSubshell(BashppParser::SubshellContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_SUBSHELL_CPP_
