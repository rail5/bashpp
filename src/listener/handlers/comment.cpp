/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_COMMENT_CPP_
#define SRC_LISTENER_HANDLERS_COMMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterComment(BashppParser::CommentContext *ctx) {
	skip_syntax_errors
	in_comment = true;
}

void BashppListener::exitComment(BashppParser::CommentContext *ctx) {
	skip_syntax_errors
	in_comment = false;
}

#endif // SRC_LISTENER_HANDLERS_COMMENT_CPP_
