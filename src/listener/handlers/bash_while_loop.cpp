/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

/**
* This file contains parser rules for all of:
* 		1. Bash_while_loop
* 		2. Bash_while_condition
*/

#ifndef SRC_LISTENER_HANDLERS_BASH_WHILE_LOOP_CPP_
#define SRC_LISTENER_HANDLERS_BASH_WHILE_LOOP_CPP_

#include "../BashppListener.h"

void BashppListener::enterBash_while_loop(BashppParser::Bash_while_loopContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error_sym(ctx->start, "While loop outside of a code entity");
	}

	std::shared_ptr<bpp::bash_while_loop> while_statement = std::make_shared<bpp::bash_while_loop>();
	while_statement->set_containing_class(current_code_entity->get_containing_class());
	while_statement->inherit(current_code_entity);

	entity_stack.push(while_statement);
}

void BashppListener::exitBash_while_loop(BashppParser::Bash_while_loopContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bash_while_loop> while_statement = std::dynamic_pointer_cast<bpp::bash_while_loop>(entity_stack.top());
	if (while_statement == nullptr) {
		throw internal_error("While statement entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack", ctx);
	}

	current_code_entity->add_code_to_previous_line(while_statement->get_while_condition()->get_pre_code());
	current_code_entity->add_code_to_next_line(while_statement->get_while_condition()->get_post_code());

	std::string supershell_evaluation = "";
	for (const std::string& function_call : while_statement->get_while_condition()->get_supershell_function_calls()) {
		supershell_evaluation += function_call + "\n";
	}

	current_code_entity->add_code_to_previous_line(supershell_evaluation); // Evaluate the supershell before the while loop starts

	current_code_entity->add_code_to_previous_line("while " + while_statement->get_while_condition()->get_code() + "; do\n");

	current_code_entity->add_code_to_previous_line(while_statement->get_pre_code());
	current_code_entity->add_code_to_next_line(while_statement->get_post_code());

	// Re-evaluate the supershell at the end of the while loop (before the next iteration)
	current_code_entity->add_code(while_statement->get_code() + "\n" + supershell_evaluation + "done\n");
}

void BashppListener::enterBash_while_condition(BashppParser::Bash_while_conditionContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bash_while_loop> while_statement = std::dynamic_pointer_cast<bpp::bash_while_loop>(entity_stack.top());
	if (while_statement == nullptr) {
		throw internal_error("While statement entity not found in the entity stack", ctx);
	}

	std::shared_ptr<bpp::bash_while_condition> while_condition = std::make_shared<bpp::bash_while_condition>();
	while_condition->set_containing_class(while_statement->get_containing_class());
	while_condition->inherit(while_statement);

	while_statement->set_while_condition(while_condition);

	entity_stack.push(while_condition);

	in_while_condition = true;
	current_while_condition = while_condition;
}

void BashppListener::exitBash_while_condition(BashppParser::Bash_while_conditionContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bash_while_condition> while_condition = std::dynamic_pointer_cast<bpp::bash_while_condition>(entity_stack.top());
	if (while_condition == nullptr) {
		throw internal_error("While condition entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	in_while_condition = false;
	current_while_condition = nullptr;
}


#endif // SRC_LISTENER_HANDLERS_BASH_WHILE_LOOP_CPP_
