/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
* This file contains parser rules for all of:
* 		1. BashForStatement
* 		2. BashSelectStatement
* 		3. BashInCondition
*/

#include <listener/BashppListener.h>

void BashppListener::enterBashForStatement(std::shared_ptr<AST::BashForStatement> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "For statement outside of code entity");
	}

	std::shared_ptr<bpp::bash_for_or_select> for_statement = std::make_shared<bpp::bash_for_or_select>();
	for_statement->set_containing_class(current_code_entity->get_containing_class());
	for_statement->inherit(current_code_entity);

	std::string header_code = "for " + node->VARIABLE().getValue() + " in ";
	for_statement->set_header_code(header_code);

	entity_stack.push(for_statement);

	for_statement->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBashForStatement(std::shared_ptr<AST::BashForStatement> node) {
	bpp_assert(topmost_entity_is<bpp::bash_for_or_select>(), "For loop entity not found in the entity stack");
	auto for_loop = std::static_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());

	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::bpp_code_entity>(), "Current code entity not found in the entity stack");
	auto current_code_entity = std::static_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	for_loop->destruct_local_objects(program);
	for_loop->flush_code_buffers();

	current_code_entity->add_code_to_previous_line(for_loop->get_header_pre_code());
	current_code_entity->add_code_to_next_line(for_loop->get_header_post_code());
	current_code_entity->add_code(for_loop->get_header_code());
	current_code_entity->add_code(for_loop->get_pre_code() + for_loop->get_code() + for_loop->get_post_code());
	current_code_entity->add_code("done ", false);

	program->mark_entity(
		source_file,
		for_loop->get_initial_definition().line,
		for_loop->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		for_loop
	);
}

void BashppListener::enterBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node) {
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Select statement outside of code entity");
	}

	auto select_statement = std::make_shared<bpp::bash_for_or_select>();
	select_statement->set_containing_class(current_code_entity->get_containing_class());
	select_statement->inherit(current_code_entity);

	std::string header_code = "select " + node->VARIABLE().getValue() + " in ";
	select_statement->set_header_code(header_code);

	entity_stack.push(select_statement);

	select_statement->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBashSelectStatement(std::shared_ptr<AST::BashSelectStatement> node) {
	bpp_assert(topmost_entity_is<bpp::bash_for_or_select>(), "Select statement entity not found in the entity stack");
	auto select_statement = std::static_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());

	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::bpp_code_entity>(), "Current code entity not found in the entity stack");
	auto current_code_entity = std::static_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	select_statement->destruct_local_objects(program);
	select_statement->flush_code_buffers();

	current_code_entity->add_code_to_previous_line(select_statement->get_header_pre_code());
	current_code_entity->add_code_to_next_line(select_statement->get_header_post_code());
	current_code_entity->add_code(select_statement->get_header_code());
	current_code_entity->add_code(select_statement->get_pre_code() + select_statement->get_code() + select_statement->get_post_code());
	current_code_entity->add_code("done ", false);


	program->mark_entity(
		source_file,
		select_statement->get_initial_definition().line,
		select_statement->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		select_statement
	);
}

void BashppListener::enterBashInCondition(std::shared_ptr<AST::BashInCondition> node) {
	bpp_assert(topmost_entity_is<bpp::bash_for_or_select>(), "For/select statement entity not found in the entity stack");
	auto parent_statement = std::static_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());

	std::shared_ptr<bpp::bpp_string> in_condition = std::make_shared<bpp::bpp_string>();
	in_condition->set_containing_class(parent_statement->get_containing_class());
	in_condition->inherit(parent_statement);
	entity_stack.push(in_condition);
}

void BashppListener::exitBashInCondition(std::shared_ptr<AST::BashInCondition> node) {
	bpp_assert(topmost_entity_is<bpp::bpp_string>(), "In condition entity not found in the entity stack");
	auto in_condition = std::static_pointer_cast<bpp::bpp_string>(entity_stack.top());

	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::bash_for_or_select>(), "For/select statement entity not found in the entity stack");
	auto parent_statement = std::static_pointer_cast<bpp::bash_for_or_select>(entity_stack.top());

	parent_statement->set_header_pre_code(in_condition->get_pre_code());
	parent_statement->set_header_post_code(in_condition->get_post_code());

	std::string header_code = parent_statement->get_header_code();
	header_code += in_condition->get_code() + "; do\n";
	parent_statement->set_header_code(header_code);
}
