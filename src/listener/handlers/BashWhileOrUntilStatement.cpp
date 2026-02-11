/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

/**
* This file contains parser rules for all of:
* 		1. BashWhileStatement
*		2. BashUntilStatement
* 		3. BashWhileOrUntilCondition
*/

#include <listener/BashppListener.h>
#include <memory>

void BashppListener::enterBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "While statement outside of code entity");
	}

	std::shared_ptr<bpp::bash_while_or_until_loop> while_statement = std::make_shared<bpp::bash_while_or_until_loop>();
	while_statement->set_containing_class(current_code_entity->get_containing_class());
	while_statement->inherit(current_code_entity);

	entity_stack.push(while_statement);

	while_statement->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBashWhileStatement(std::shared_ptr<AST::BashWhileStatement> node) {
	std::shared_ptr<bpp::bash_while_or_until_loop> while_statement = std::dynamic_pointer_cast<bpp::bash_while_or_until_loop>(entity_stack.top());
	if (while_statement == nullptr) {
		throw bpp::ErrorHandling::InternalError("While statement entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(while_statement->get_condition()->get_pre_code());
	current_code_entity->add_code_to_next_line(while_statement->get_condition()->get_post_code());

	current_code_entity->add_code("while " + while_statement->get_condition()->get_code() + "; do\n"
		+ while_statement->get_pre_code() + "\n"
		+ while_statement->get_code() + "\ndone", false);

	program->mark_entity(
		source_file,
		while_statement->get_initial_definition().line,
		while_statement->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		while_statement
	);
}

void BashppListener::enterBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Until statement outside of code entity");
	}

	std::shared_ptr<bpp::bash_while_or_until_loop> until_statement = std::make_shared<bpp::bash_while_or_until_loop>();
	until_statement->set_containing_class(current_code_entity->get_containing_class());
	until_statement->inherit(current_code_entity);

	entity_stack.push(until_statement);

	until_statement->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBashUntilStatement(std::shared_ptr<AST::BashUntilStatement> node) {
	std::shared_ptr<bpp::bash_while_or_until_loop> until_statement = std::dynamic_pointer_cast<bpp::bash_while_or_until_loop>(entity_stack.top());
	if (until_statement == nullptr) {
		throw bpp::ErrorHandling::InternalError("Until statement entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(until_statement->get_condition()->get_pre_code());
	current_code_entity->add_code_to_next_line(until_statement->get_condition()->get_post_code());

	// If we're targeting Bash 5.2 or earlier, extra logic is needed to re-evaluate supershells with each iteration of the loop
	// If we're targeting Bash 5.3 or later, we can use the new native implementation
	current_code_entity->add_code("until " + until_statement->get_condition()->get_code() + "; do\n"
		+ until_statement->get_pre_code() + "\n"
		+ until_statement->get_code() + "\ndone", false);

	program->mark_entity(
		source_file,
		until_statement->get_initial_definition().line,
		until_statement->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		until_statement
	);
}

void BashppListener::enterBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node) {
	std::shared_ptr<bpp::bash_while_or_until_loop> loop = std::dynamic_pointer_cast<bpp::bash_while_or_until_loop>(entity_stack.top());
	if (loop == nullptr) {
		throw bpp::ErrorHandling::InternalError("While/until statement entity not found in the entity stack");
	}

	std::shared_ptr<bpp::bash_while_or_until_condition> condition = std::make_shared<bpp::bash_while_or_until_condition>();
	condition->set_containing_class(loop->get_containing_class());
	condition->inherit(loop);

	loop->set_condition(condition);

	entity_stack.push(condition);
}

void BashppListener::exitBashWhileOrUntilCondition(std::shared_ptr<AST::BashWhileOrUntilCondition> node) {
	std::shared_ptr<bpp::bash_while_or_until_condition> condition = std::dynamic_pointer_cast<bpp::bash_while_or_until_condition>(entity_stack.top());
	if (condition == nullptr) {
		throw bpp::ErrorHandling::InternalError("While/until condition entity not found in the entity stack");
	}

	entity_stack.pop();
}

