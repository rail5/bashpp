/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_SUBSHELL_CPP_
#define SRC_LISTENER_HANDLERS_SUBSHELL_CPP_

#include "../BashppListener.h"

void BashppListener::enterSubshell(BashppParser::SubshellContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw_syntax_error(ctx->SUBSHELL_START(), "Subshell outside of code entity");
	}

	// Create a new code entity for the subshell
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::make_shared<bpp::bpp_string>();
	subshell_entity->set_containing_class(code_entity->get_containing_class());
	subshell_entity->inherit(code_entity);

	// Push the subshell entity onto the entity stack
	entity_stack.push(subshell_entity);
}

void BashppListener::exitSubshell(BashppParser::SubshellContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> subshell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (subshell_entity == nullptr) {
		throw internal_error("Subshell context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	current_code_entity->add_code(ctx->SUBSHELL_START()->getText());
	current_code_entity->add_code(subshell_entity->get_pre_code());
	current_code_entity->add_code(subshell_entity->get_code());
	if (!subshell_entity->get_post_code().empty()) {
		current_code_entity->add_code("\n");
	}
	current_code_entity->add_code(subshell_entity->get_post_code());
	current_code_entity->add_code(ctx->SUBSHELL_END()->getText());
}

#endif // SRC_LISTENER_HANDLERS_SUBSHELL_CPP_
