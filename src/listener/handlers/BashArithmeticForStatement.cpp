/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "For statement outside of code entity");
	}

	std::shared_ptr<bpp::bash_for_or_select> for_statement = std::make_shared<bpp::bash_for_or_select>();
	for_statement->set_containing_class(current_code_entity->get_containing_class());
	for_statement->inherit(current_code_entity);
	entity_stack.push(for_statement);
}

void BashppListener::exitBashArithmeticForStatement(std::shared_ptr<AST::BashArithmeticForStatement> node) {
	skip_syntax_errors
	auto for_statement = std::dynamic_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());
	if (for_statement == nullptr) {
		throw internal_error("For loop entity not found in the entity stack");
	}
	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(for_statement->get_header_pre_code());
	current_code_entity->add_code_to_next_line("done\n");
	current_code_entity->add_code_to_next_line(for_statement->get_header_post_code());
	current_code_entity->add_code_to_previous_line(for_statement->get_header_code());
	current_code_entity->add_code(for_statement->get_pre_code() + for_statement->get_code() + for_statement->get_post_code());

	program->mark_entity(
		source_file,
		node->getLine(),
		node->getCharPositionInLine(),
		node->getEndPosition().line,
		node->getEndPosition().column,
		for_statement
	);
}

void BashppListener::enterBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node) {
	skip_syntax_errors
	auto for_statement = std::dynamic_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());
	if (for_statement == nullptr) {
		throw internal_error("For condition outside of for/select statement");
	}

	std::shared_ptr<bpp::bpp_string> for_condition = std::make_shared<bpp::bpp_string>();
	for_condition->set_containing_class(for_statement->get_containing_class());
	for_condition->inherit(for_statement);
	entity_stack.push(for_condition);
}

void BashppListener::exitBashArithmeticForCondition(std::shared_ptr<AST::BashArithmeticForCondition> node) {
	skip_syntax_errors
	auto condition = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (condition == nullptr) {
		throw internal_error("For condition entity not found in the entity stack");
	}

	entity_stack.pop();

	auto for_statement = std::dynamic_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());
	if (for_statement == nullptr) {
		throw internal_error("For/select statement entity not found in the entity stack");
	}

	for_statement->set_header_pre_code(condition->get_pre_code());
	for_statement->set_header_post_code(condition->get_post_code());
	for_statement->set_header_code("for ((" + condition->get_code() + ")) ; do\n");
}

// No need to handle BashArithmeticStatement specially,
// Since they're StringType nodes, they have inner nodes which will call ->add_code()
// on the containing BashArithmeticForCondition automatically
