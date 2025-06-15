/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

/**
* This file contains parser rules for all of:
* 		1. Bash_for_loop
* 		2. Bash_for_header
*/

#include "../BashppListener.h"

void BashppListener::enterBash_for_loop(BashppParser::Bash_for_loopContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error_sym(ctx->start, "For loop outside of a code entity");
	}

	std::shared_ptr<bpp::bash_for> for_statement = std::make_shared<bpp::bash_for>();
	for_statement->set_containing_class(current_code_entity->get_containing_class());
	for_statement->inherit(current_code_entity);

	entity_stack.push(for_statement);
}

void BashppListener::exitBash_for_loop(BashppParser::Bash_for_loopContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_for> for_loop = std::dynamic_pointer_cast<bpp::bash_for>(entity_stack.top());
	if (for_loop == nullptr) {
		throw internal_error("For loop entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack", ctx);
	}

	current_code_entity->add_code_to_previous_line(for_loop->get_header_pre_code());
	current_code_entity->add_code_to_next_line("done\n");
	current_code_entity->add_code_to_next_line(for_loop->get_header_post_code());
	current_code_entity->add_code_to_previous_line(for_loop->get_header_code());
	current_code_entity->add_code(for_loop->get_pre_code() + for_loop->get_code() + for_loop->get_post_code());
}

void BashppListener::enterBash_for_header(BashppParser::Bash_for_headerContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_for> for_loop = std::dynamic_pointer_cast<bpp::bash_for>(entity_stack.top());
	if (for_loop == nullptr) {
		throw internal_error("For loop header outside of a for loop", ctx);
	}

	std::shared_ptr<bpp::bpp_string> for_header = std::make_shared<bpp::bpp_string>();
	for_header->set_containing_class(for_loop->get_containing_class());
	for_header->inherit(for_loop);

	entity_stack.push(for_header);
}

void BashppListener::exitBash_for_header(BashppParser::Bash_for_headerContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> for_header = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (for_header == nullptr) {
		throw internal_error("For loop header entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_for> for_loop = std::dynamic_pointer_cast<bpp::bash_for>(entity_stack.top());
	if (for_loop == nullptr) {
		throw internal_error("For loop entity not found in the entity stack", ctx);
	}

	for_loop->set_header_pre_code(for_header->get_pre_code());
	for_loop->set_header_post_code(for_header->get_post_code());

	// Generate the header
	// Standard case:
	//		for identifier in expression; do
	// Arithmetic case:
	//		for ((expression)); do
	std::string for_header_code = "for ";
	if (ctx->IDENTIFIER() != nullptr) {
		for_header_code += ctx->IDENTIFIER()->getText() + " in ";
	}

	for_header_code += for_header->get_code() + "; do\n";

	for_loop->set_header_code(for_header_code);
}

