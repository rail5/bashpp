/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_POINTER_DEREFERENCE_CPP_
#define SRC_LISTENER_HANDLERS_POINTER_DEREFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Pointer dereferences take the form
	 * 	*@IDENTIFIER.IDENTIFIER...
	 * Or
	 * 	*@this.IDENTIFIER.IDENTIFIER...
	 * The result should be the object pointed to by the pointer
	 * For instance,
	 *  @MyClass myObject=*@myPointer
	 * Should copy the object pointed to by myPointer into myObject
	 * Or:
	 *  var=*@myPointer
	 * Should store the output of the object's toPrimitive method in var
	 */

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->ASTERISK(), "Pointer dereference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference_entity = std::make_shared<bpp::bpp_pointer_dereference>();
	pointer_dereference_entity->set_containing_class(current_code_entity->get_containing_class());
	pointer_dereference_entity->inherit(current_code_entity);

	// Are we in a value assignment context?
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());

	if (value_assignment != nullptr) {
		pointer_dereference_entity->set_value_assignment(value_assignment);
	}

	entity_stack.push(pointer_dereference_entity);
}

void BashppListener::exitPointer_dereference(BashppParser::Pointer_dereferenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference_entity = std::dynamic_pointer_cast<bpp::bpp_pointer_dereference>(entity_stack.top());

	if (pointer_dereference_entity == nullptr) {
		throw internal_error("Pointer dereference context not found in entity stack");
	}

	entity_stack.pop();

	// Are we in a value assignment context?
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());

	if (value_assignment != nullptr) {
		return;
	}

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Code entity not found on the entity stack");
	}

	current_code_entity->add_code_to_previous_line(pointer_dereference_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(pointer_dereference_entity->get_post_code());
	current_code_entity->add_code(pointer_dereference_entity->get_code());
}

#endif // SRC_LISTENER_HANDLERS_POINTER_DEREFERENCE_CPP_
