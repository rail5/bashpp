/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_
#define ANTLR_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_
