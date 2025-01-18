/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_OBJECT_ADDRESS_CPP_
#define SRC_LISTENER_HANDLERS_OBJECT_ADDRESS_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_address(BashppParser::Object_addressContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitObject_address(BashppParser::Object_addressContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_OBJECT_ADDRESS_CPP_
