/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_NEW_STATEMENT_CPP_
#define SRC_LISTENER_HANDLERS_NEW_STATEMENT_CPP_

#include "../BashppListener.h"

void BashppListener::enterNew_statement(BashppParser::New_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * New statements take the form
	 * 	@new ClassName
	 * Where ClassName is the name of the class to instantiate
	 * 
	 * This statement creates a new object of the specified class
	 * And replaces the "@new ClassName" statement with the address of the new object
	*/

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->AT(0), "New statement outside of code entity");
	}

	// Verify that the class actually exists
	std::string class_name = ctx->IDENTIFIER()->getText();
	std::shared_ptr<bpp::bpp_class> new_class = current_code_entity->get_class(class_name);

	if (new_class == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(), "Class not found: " + class_name);
	}

	code_segment new_object_code = generate_new_code(new_class);

	current_code_entity->add_code_to_previous_line(new_object_code.pre_code);
	current_code_entity->add_code_to_next_line(new_object_code.post_code);
	current_code_entity->add_code(new_object_code.code);
}

void BashppListener::exitNew_statement(BashppParser::New_statementContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string
}

#endif // SRC_LISTENER_HANDLERS_NEW_STATEMENT_CPP_
