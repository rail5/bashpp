/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_
#define ANTLR_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_OBJECT_ASSIGNMENT_CPP_
