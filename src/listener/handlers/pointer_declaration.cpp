/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_POINTER_DECLARATION_CPP_
#define SRC_LISTENER_HANDLERS_POINTER_DECLARATION_CPP_

#include "../BashppListener.h"

void BashppListener::enterPointer_declaration(BashppParser::Pointer_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitPointer_declaration(BashppParser::Pointer_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_POINTER_DECLARATION_CPP_
