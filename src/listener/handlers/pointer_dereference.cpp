/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_POINTER_DEREFERENCE_CPP_
#define ANTLR_LISTENER_HANDLERS_POINTER_DEREFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_POINTER_DEREFERENCE_CPP_
