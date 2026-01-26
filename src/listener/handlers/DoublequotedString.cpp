/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterDoublequotedString(std::shared_ptr<AST::DoublequotedString> node) {
	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "String outside of code entity");
	}

	std::shared_ptr<bpp::bpp_string> string_code_entity = std::make_shared<bpp::bpp_string>();
	string_code_entity->set_containing_class(current_class);
	string_code_entity->inherit(current_code_entity);
	entity_stack.push(string_code_entity);

	// If we're not in a broader context, simply add an open quote to the current code entity
	if (current_code_entity != nullptr) {
		current_code_entity->add_code("\"", false);
		return;
	}
}

void BashppListener::exitDoublequotedString(std::shared_ptr<AST::DoublequotedString> node) {
	std::shared_ptr<bpp::bpp_code_entity> string_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	entity_stack.pop();

	if (string_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("String context was not found in the entity stack");
	}

	// If we're not in a broader context, simply add the current string contents + a close quote to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(string_code_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(string_code_entity->get_post_code());
		current_code_entity->add_code(string_code_entity->get_code() + "\"", false);
		return;
	}
}
