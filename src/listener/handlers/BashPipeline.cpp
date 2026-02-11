/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterBashPipeline(std::shared_ptr<AST::BashPipeline> node) {
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Command outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> pipeline = std::make_shared<bpp::bpp_string>();
	pipeline->set_containing_class(current_code_entity->get_containing_class());
	pipeline->inherit(current_code_entity);
	entity_stack.push(pipeline);
}

void BashppListener::exitBashPipeline(std::shared_ptr<AST::BashPipeline> node) {
	auto pipeline = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (pipeline == nullptr) {
		throw bpp::ErrorHandling::InternalError("Pipeline context was not found in the entity stack");
	}

	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(pipeline->get_pre_code());
	current_code_entity->add_code_to_next_line(pipeline->get_post_code());
	current_code_entity->add_code(pipeline->get_code());
	
	// Pass any instantiated objects etc up the chain
	// (A pipeline is not a closed scope)
	current_code_entity->inherit(pipeline);
}
