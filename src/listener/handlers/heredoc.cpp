/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

/**
* This file contains parser rules for all of:
* 		1. Heredoc
* 		2. Heredoc_header
*/

#ifndef SRC_LISTENER_HANDLERS_HEREDOC_CPP_
#define SRC_LISTENER_HANDLERS_HEREDOC_CPP_

#include "../BashppListener.h"

void BashppListener::enterHeredoc(BashppParser::HeredocContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error_sym(ctx->start, "Heredoc outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::make_shared<bpp::bpp_string>();
	heredoc_entity->set_containing_class(current_code_entity->get_containing_class());
	heredoc_entity->inherit(current_code_entity);

	entity_stack.push(heredoc_entity);
}

void BashppListener::exitHeredoc(BashppParser::HeredocContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (heredoc_entity == nullptr) {
		throw internal_error("Heredoc entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(heredoc_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(heredoc_entity->get_post_code());
	current_code_entity->add_code(heredoc_entity->get_code() + ctx->HEREDOC_END()->getText(), false);
}

void BashppListener::enterHeredoc_header(BashppParser::Heredoc_headerContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (heredoc_entity == nullptr) {
		throw internal_error("Heredoc entity not found in the entity stack");
	}

	std::shared_ptr<bpp::bpp_string> heredoc_header_entity = std::make_shared<bpp::bpp_string>();
	heredoc_header_entity->set_containing_class(heredoc_entity->get_containing_class());
	heredoc_header_entity->inherit(heredoc_entity);

	entity_stack.push(heredoc_header_entity);
}

void BashppListener::exitHeredoc_header(BashppParser::Heredoc_headerContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> heredoc_header_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (heredoc_header_entity == nullptr) {
		throw internal_error("Heredoc header entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_string> heredoc_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (heredoc_entity == nullptr) {
		throw internal_error("Heredoc entity not found in the entity stack");
	}

	heredoc_entity->add_code_to_previous_line(heredoc_header_entity->get_pre_code());
	heredoc_entity->add_code_to_next_line(heredoc_header_entity->get_post_code());
	heredoc_entity->add_code(ctx->HEREDOC_START()->getText() + heredoc_header_entity->get_code() + ctx->HEREDOC_CONTENT()->getText());
}

#endif // SRC_LISTENER_HANDLERS_HEREDOC_CPP_
