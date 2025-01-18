/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_
#define SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterConstructor_definition(BashppParser::Constructor_definitionContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

void BashppListener::exitConstructor_definition(BashppParser::Constructor_definitionContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_CONSTRUCTOR_DEFINITION_CPP_
