/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) {
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Command sequence outside of a code entity");
	}

	std::shared_ptr<bpp::bash_command_sequence> command_sequence_entity = std::make_shared<bpp::bash_command_sequence>();
	command_sequence_entity->set_containing_class(current_code_entity->get_containing_class());
	command_sequence_entity->inherit(current_code_entity);
	command_sequence_entity->set_perfect_forwarding(current_code_entity->get_requires_perfect_forwarding());
	entity_stack.push(command_sequence_entity);
}

void BashppListener::exitBashCommandSequence(std::shared_ptr<AST::BashCommandSequence> node) {
	auto command_sequence_entity = std::dynamic_pointer_cast<bpp::bash_command_sequence>(entity_stack.top());
	if (command_sequence_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Bash command sequence context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(command_sequence_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(command_sequence_entity->get_post_code());
	current_code_entity->add_code(command_sequence_entity->get_code());

	// Pass any instantiated objects etc up the chain
	// (A command sequence is not a closed scope)
	current_code_entity->inherit(command_sequence_entity);
}
