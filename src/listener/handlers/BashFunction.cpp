/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBashFunction(std::shared_ptr<AST::BashFunction> node) {
	skip_syntax_errors
	/**
	 * Bash functions take the format:
	 * 
	 * function function_name { ... }
	 *
	 * Or:
	 * function function_name() { ... }
	 *
	 * Or:
	 * function_name() { ... }
	 */

	bash_function_stack.push({});

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Function definition outside of code entity");
	}

	// What's the name of the function?
	auto function_name_token = node->NAME();
	
	// Create a new code entity for the function
	std::shared_ptr<bpp::bash_function> function_entity = std::make_shared<bpp::bash_function>();
	function_entity->set_name(function_name_token.getValue());
	function_entity->set_containing_class(current_code_entity->get_containing_class());
	function_entity->inherit(current_code_entity);

	// Push the entity onto the stack
	entity_stack.push(function_entity);

	function_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBashFunction(std::shared_ptr<AST::BashFunction> node) {
	skip_syntax_errors

	std::shared_ptr<bpp::bash_function> function_entity = std::dynamic_pointer_cast<bpp::bash_function>(entity_stack.top());
	if (function_entity == nullptr) {
		throw internal_error("Function context was not found in the entity stack");
	}
	entity_stack.pop();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Couldn't find the current code entity");
	}

	// Add the function to the current code entity
	current_code_entity->add_code_to_previous_line("function " + function_entity->get_name() + " {\n");
	current_code_entity->add_code_to_next_line("}");

	function_entity->flush_code_buffers();
	
	std::string code_to_add = function_entity->get_pre_code();
	if (!code_to_add.empty()) {
		code_to_add += "\n";
	}
	code_to_add += function_entity->get_code();
	if (!function_entity->get_post_code().empty()) {
		code_to_add += "\n" + function_entity->get_post_code();
	}
	current_code_entity->add_code(code_to_add);

	program->mark_entity(
		source_file,
		function_entity->get_initial_definition().line,
		function_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		function_entity
	);

	bash_function_stack.pop();
}
