/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_NULLPTR_REF_CPP_
#define SRC_LISTENER_HANDLERS_NULLPTR_REF_CPP_

#include "../BashppListener.h"

void BashppListener::enterNullptr_ref(BashppParser::Nullptr_refContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Nullptr references take the form
	 * 	@nullptr
	 * 
	 * This reference will be replaced with the value of the nullptr constant
	 */

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->KEYWORD_NULLPTR(), "Nullptr reference outside of code entity");
	}

	current_code_entity->add_code(bpp::bpp_nullptr);
}

void BashppListener::exitNullptr_ref(BashppParser::Nullptr_refContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_NULLPTR_REF_CPP_
