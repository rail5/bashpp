/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_SUPERSHELL_CPP_
#define ANTLR_LISTENER_HANDLERS_SUPERSHELL_CPP_

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

#endif // ANTLR_LISTENER_HANDLERS_SUPERSHELL_CPP_
