/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <listener/BashppListener.h>

void BashppListener::enterBlock(std::shared_ptr<AST::Block> node) {
	/**
	 * Blocks are just groups of statements enclosed in curly-braces
	 * Sometimes, a block is a part of a larger construct, such as a method or class definition
	 */

	// Special cases: If this block is a part of a larger construct, do not add surrounding braces
	auto parent_entity = entity_stack.top();
	auto surrounding_class = std::dynamic_pointer_cast<bpp::bpp_class>(parent_entity);
	auto surrounding_method = std::dynamic_pointer_cast<bpp::bpp_method>(parent_entity);
	auto surrounding_bash_function = std::dynamic_pointer_cast<bpp::bash_function>(parent_entity);
	auto surrounding_bash_for_or_select_statement = std::dynamic_pointer_cast<bpp::bash_for_or_select>(parent_entity);

	bool is_surrounded_by_larger_construct =
		surrounding_class != nullptr ||
		surrounding_method != nullptr ||
		surrounding_bash_function != nullptr ||
		surrounding_bash_for_or_select_statement != nullptr;

	if (is_surrounded_by_larger_construct) return;

	// If however we're not surrounded by a larger construct, add the surrounding braces
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Statement block outside of code entity");
	}

	// Preface the block with an opening curly brace
	// TODO(@rail5): Does the curly-brace properly belong as part of the block entity rather than its parent code entity?
	current_code_entity->add_code("{\n", false);

	auto block_entity = std::make_shared<bpp::bpp_code_entity>();
	block_entity->inherit(current_code_entity);
	block_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);

	entity_stack.push(block_entity);
}

void BashppListener::exitBlock(std::shared_ptr<AST::Block> node) {
	auto parent_entity = entity_stack.top();
	auto surrounding_class = std::dynamic_pointer_cast<bpp::bpp_class>(parent_entity);
	auto surrounding_method = std::dynamic_pointer_cast<bpp::bpp_method>(parent_entity);
	auto surrounding_bash_function = std::dynamic_pointer_cast<bpp::bash_function>(parent_entity);
	auto surrounding_bash_for_or_select_statement = std::dynamic_pointer_cast<bpp::bash_for_or_select>(parent_entity);

	bool is_surrounded_by_larger_construct =
		surrounding_class != nullptr ||
		surrounding_method != nullptr ||
		surrounding_bash_function != nullptr ||
		surrounding_bash_for_or_select_statement != nullptr;

	if (is_surrounded_by_larger_construct) return;

	auto block_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	bpp_assert(block_entity != nullptr, "Block context was not found in the entity stack");
	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	bpp_assert(current_code_entity != nullptr, "Containing code entity was not found in the entity stack");

	block_entity->destruct_local_objects(program);
	block_entity->flush_code_buffers();
	
	current_code_entity->add_code(block_entity->get_pre_code());
	current_code_entity->add_code(block_entity->get_code());
	current_code_entity->add_code(block_entity->get_post_code());

	current_code_entity->add_code("\n}", false);
}
