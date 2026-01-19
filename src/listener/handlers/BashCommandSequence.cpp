/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) {
	skip_syntax_errors

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Command sequence outside of a code entity");
	}

	std::shared_ptr<bpp::bash_command_sequence> command_sequence_entity = std::make_shared<bpp::bash_command_sequence>();
	command_sequence_entity->set_containing_class(current_code_entity->get_containing_class());
	command_sequence_entity->inherit(current_code_entity);
	entity_stack.push(command_sequence_entity);
}

void BashppListener::exitBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) {
	skip_syntax_errors

	auto command_sequence_entity = std::dynamic_pointer_cast<bpp::bash_command_sequence>(entity_stack.top());
	if (command_sequence_entity == nullptr) {
		throw internal_error("Bash command sequence context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack");
	}

	// The pre-code and post-code for the command sequence have already been joined into the sequence's main code buffer
	// Therefore no need to call add_code_to_previous_line or add_code_to_next_line here
	current_code_entity->add_code(command_sequence_entity->get_code());
}
