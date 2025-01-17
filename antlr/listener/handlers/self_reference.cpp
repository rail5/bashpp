/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
#define ANTLR_LISTENER_HANDLERS_SELF_REFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
