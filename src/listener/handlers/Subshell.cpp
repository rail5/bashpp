/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) {
	skip_syntax_errors
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw_syntax_error(node, "Subshell substitution outside of code entity");
	}

	// Create a new code entity for the subshell
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::make_shared<bpp::bpp_string>();
	subshell_entity->set_containing_class(code_entity->get_containing_class());
	subshell_entity->inherit(code_entity);

	// Push the subshell entity onto the entity stack
	entity_stack.push(subshell_entity);

	subshell_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (subshell_entity == nullptr) {
		throw internal_error("Subshell substitution context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	current_code_entity->add_code_to_previous_line(subshell_entity->get_pre_code());
	current_code_entity->add_code_to_next_line("\n" + subshell_entity->get_post_code());
	current_code_entity->add_code("$(" + subshell_entity->get_code() + ")");

	program->mark_entity(
		source_file,
		subshell_entity->get_initial_definition().line,
		subshell_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		subshell_entity
	);
}

void BashppListener::enterRawSubshell(std::shared_ptr<AST::RawSubshell> node) {
	skip_syntax_errors
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw_syntax_error(node, "Subshell outside of code entity");
	}

	// Create a new code entity for the subshell
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::make_shared<bpp::bpp_string>();
	subshell_entity->set_containing_class(code_entity->get_containing_class());
	subshell_entity->inherit(code_entity);

	// Push the subshell entity onto the entity stack
	entity_stack.push(subshell_entity);

	subshell_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitRawSubshell(std::shared_ptr<AST::RawSubshell> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (subshell_entity == nullptr) {
		throw internal_error("Subshell context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	auto value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());
	if (value_assignment_entity != nullptr) {
		// In we're in a value assignment, this is not in fact a subshell, but an array assignment
		// E.g.: var=() or var+=("abc")
		value_assignment_entity->set_array_assignment(true);
	}

	current_code_entity->add_code_to_previous_line(subshell_entity->get_pre_code());
	current_code_entity->add_code_to_next_line("\n" + subshell_entity->get_post_code());
	current_code_entity->add_code("(" + subshell_entity->get_code() + ")");

	program->mark_entity(
		source_file,
		subshell_entity->get_initial_definition().line,
		subshell_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		subshell_entity
	);
}
