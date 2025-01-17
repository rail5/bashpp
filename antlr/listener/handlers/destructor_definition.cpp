/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_
#define ANTLR_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_DESTRUCTOR_DEFINITION_CPP_
