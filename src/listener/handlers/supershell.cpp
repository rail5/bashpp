/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_SUPERSHELL_CPP_
#define SRC_LISTENER_HANDLERS_SUPERSHELL_CPP_

#include "../BashppListener.h"

void BashppListener::enterSupershell(BashppParser::SupershellContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitSupershell(BashppParser::SupershellContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_SUPERSHELL_CPP_
