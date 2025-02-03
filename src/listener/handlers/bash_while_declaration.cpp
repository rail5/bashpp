/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_BASH_WHILE_DECLARATION_CPP_
#define SRC_LISTENER_HANDLERS_BASH_WHILE_DECLARATION_CPP_

#include "../BashppListener.h"

void BashppListener::enterBash_while_declaration(BashppParser::Bash_while_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Bash while declarations take the form
	 * 	@while (condition)
	 * Where condition is a boolean expression
	 * 
	 * This rule should only ever be caught inside a code_entity context
	 * 
	 * The only reason we catch it here at all is to implement our hacky fix to ensure that
	 * 	Supershells are re-evaluated on each iteration of the while loop
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->BASH_KEYWORD_WHILE(), "'While' loop outside of code entity");
	}

	// Create a new code entity for the while loop
	std::shared_ptr<bpp::bash_while> while_entity = std::make_shared<bpp::bash_while>();
	while_entity->set_containing_class(current_code_entity->get_containing_class());
	while_entity->inherit(current_code_entity);
	entity_stack.push(while_entity);

	in_while_statement = true;
	current_while_statement = while_entity;
}

void BashppListener::exitBash_while_declaration(BashppParser::Bash_while_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	in_while_statement = false;
	current_while_statement = nullptr;

	std::shared_ptr<bpp::bash_while> while_entity = std::dynamic_pointer_cast<bpp::bash_while>(entity_stack.top());
	if (while_entity == nullptr) {
		throw internal_error("While loop context was not found in the entity stack");
	}

	entity_stack.pop();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Code entity was not found in the entity stack");
	}

	std::string while_code = "while";

	for (int i = 0; i < while_entity->get_supershell_count(); i++) {
		while_code += " " + while_entity->get_supershell_function_calls().at(i) + " &&";
	}

	while_code += while_entity->get_code();
	while_code += ctx->BASH_WHILE_END()->getText();

	// Add the while loop to the current code entity
	current_code_entity->add_code_to_previous_line(while_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(while_entity->get_post_code());
	current_code_entity->add_code(while_code);
}

#endif // SRC_LISTENER_HANDLERS_BASH_WHILE_DECLARATION_CPP_
