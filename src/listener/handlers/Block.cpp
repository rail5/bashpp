/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
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

	current_code_entity->add_code("{\n");
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

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Statement block outside of code entity");
	}

	// Add the substitution end token to the current code entity
	current_code_entity->add_code("\n}", false);
}
