/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_PARAMETER_CPP_
#define ANTLR_LISTENER_HANDLERS_PARAMETER_CPP_

#include "../BashppListener.h"

void BashppListener::enterParameter(BashppParser::ParameterContext *ctx) {
	skip_comment
	skip_singlequote_string
}

void BashppListener::exitParameter(BashppParser::ParameterContext *ctx) {
	skip_comment
	skip_singlequote_string
}

#endif // ANTLR_LISTENER_HANDLERS_PARAMETER_CPP_
