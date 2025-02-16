/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_OBJECT_ADDRESS_CPP_
#define SRC_LISTENER_HANDLERS_OBJECT_ADDRESS_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_address(BashppParser::Object_addressContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Object addresses take the form
	 * 	&@IDENTIFIER.IDENTIFIER...
	 * Or
	 * 	&@this.IDENTIFIER.IDENTIFIER...
	 * 
	 * This reference will be replaced with the memory address of the object
	 */

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error_sym(ctx->start, "Object address outside of code entity");
	}

	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::make_shared<bpp::bpp_object_address>();
	object_address_entity->set_containing_class(current_code_entity->get_containing_class());
	object_address_entity->inherit(current_code_entity);
	entity_stack.push(object_address_entity);
}

void BashppListener::exitObject_address(BashppParser::Object_addressContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::dynamic_pointer_cast<bpp::bpp_object_address>(entity_stack.top());
	if (object_address_entity == nullptr) {
		throw internal_error("Object address context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	// Add the object address to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack", ctx);
	}

	current_code_entity->add_code_to_previous_line(object_address_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(object_address_entity->get_post_code());
	current_code_entity->add_code(object_address_entity->get_code());
}

#endif // SRC_LISTENER_HANDLERS_OBJECT_ADDRESS_CPP_
