/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_SELF_REFERENCE_AS_LVALUE_CPP_
#define SRC_LISTENER_HANDLERS_SELF_REFERENCE_AS_LVALUE_CPP_

#include "../BashppListener.h"

void BashppListener::enterSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_SELF_REFERENCE_AS_LVALUE_CPP_
