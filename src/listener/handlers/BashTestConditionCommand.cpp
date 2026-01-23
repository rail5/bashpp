/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBashTestConditionCommand(std::shared_ptr<AST::BashTestConditionCommand> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(node, "Test command outside of code entity");
	}

	std::shared_ptr<bpp::bpp_string> test_command = std::make_shared<bpp::bpp_string>();
	test_command->set_containing_class(current_class);
	test_command->inherit(current_code_entity);
	test_command->set_requires_perfect_forwarding(true);
	entity_stack.push(test_command);

	// If we're not in a broader context, simply add an open quote to the current code entity
	if (current_code_entity != nullptr) {
		current_code_entity->add_code("[[ ", false);
		return;
	}
}

void BashppListener::exitBashTestConditionCommand(std::shared_ptr<AST::BashTestConditionCommand> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_code_entity> test_command = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	entity_stack.pop();

	if (test_command == nullptr) {
		throw internal_error("Test command context was not found in the entity stack");
	}

	// If we're not in a broader context, simply add the current string contents + a close quote to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(test_command->get_pre_code());
		current_code_entity->add_code_to_next_line(test_command->get_post_code());
		current_code_entity->add_code(test_command->get_code() + " ]]", false);
		return;
	}
}
