/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBash_function(BashppParser::Bash_functionContext *ctx) {
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

	// What's the first token in the statement?
	antlr4::tree::TerminalNode* first_token = ctx->BASH_KEYWORD_FUNCTION() != nullptr ? ctx->BASH_KEYWORD_FUNCTION() : ctx->IDENTIFIER_LVALUE();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(first_token, "Function definition outside of code entity");
	}

	// What's the name of the function?
	antlr4::tree::TerminalNode* function_name_node = ctx->IDENTIFIER_LVALUE() != nullptr ? ctx->IDENTIFIER_LVALUE() : ctx->IDENTIFIER();
	if (function_name_node == nullptr) {
		throw internal_error("Function name not found in function definition", ctx);
	}
	
	// Create a new code entity for the function
	std::shared_ptr<bpp::bash_function> function_entity = std::make_shared<bpp::bash_function>();
	function_entity->set_name(function_name_node->getText());
	function_entity->set_containing_class(current_code_entity->get_containing_class());
	function_entity->inherit(current_code_entity);

	// Push the entity onto the stack
	entity_stack.push(function_entity);
}

void BashppListener::exitBash_function(BashppParser::Bash_functionContext *ctx) {
	skip_syntax_errors

	std::shared_ptr<bpp::bash_function> function_entity = std::dynamic_pointer_cast<bpp::bash_function>(entity_stack.top());
	if (function_entity == nullptr) {
		throw internal_error("Function context was not found in the entity stack", ctx);
	}
	entity_stack.pop();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Couldn't find the current code entity", ctx);
	}

	// Add the function to the current code entity
	current_code_entity->add_code_to_previous_line("function " + function_entity->get_name() + " {");
	current_code_entity->add_code_to_next_line("}");
	
	std::string code_to_add = function_entity->get_pre_code();
	if (!code_to_add.empty()) {
		code_to_add += "\n";
	}
	code_to_add += function_entity->get_code();
	if (!function_entity->get_post_code().empty()) {
		code_to_add += "\n" + function_entity->get_post_code();
	}
	current_code_entity->add_code(code_to_add);
}
