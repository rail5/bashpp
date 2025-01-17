/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_NULLPTR_REF_CPP_
#define ANTLR_LISTENER_HANDLERS_NULLPTR_REF_CPP_

#include "../BashppListener.h"

void BashppListener::enterNullptr_ref(BashppParser::Nullptr_refContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitNullptr_ref(BashppParser::Nullptr_refContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_NULLPTR_REF_CPP_
