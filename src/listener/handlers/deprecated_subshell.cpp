/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_DEPRECATED_SUBSHELL_CPP_
#define SRC_LISTENER_HANDLERS_DEPRECATED_SUBSHELL_CPP_

#include "../BashppListener.h"

void BashppListener::enterDeprecated_subshell(BashppParser::Deprecated_subshellContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitDeprecated_subshell(BashppParser::Deprecated_subshellContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_DEPRECATED_SUBSHELL_CPP_
