/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterArray_index(BashppParser::Array_indexContext *ctx) {
	skip_syntax_errors
	/**
	 * Array indices take the form:
	 * 	[...]
	 * Where the contents of the brackets are the array index
	 * And can be any valid sequence of statements
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack", ctx);
	}

	// Create a new code entity for the array index
	std::shared_ptr<bpp::bpp_string> array_index_entity = std::make_shared<bpp::bpp_string>();

	// Inherit the current code entity
	array_index_entity->set_containing_class(current_code_entity->get_containing_class());
	array_index_entity->inherit(current_code_entity);

	// Push the entity onto the stack
	entity_stack.push(array_index_entity);
}

void BashppListener::exitArray_index(BashppParser::Array_indexContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> array_index_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (array_index_entity == nullptr) {
		throw internal_error("Array index context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	// Verify that we're in an object_reference context

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());
	if (object_reference_entity == nullptr) {
		throw internal_error("Object reference entity not found on the entity stack", ctx);
	}

	// Add the array index to the object reference code if and only if the brace tokens are set
	bool has_brace = false;
	BashppParser::Ref_lvalueContext* parent_of_parent_lvalue = dynamic_cast<BashppParser::Ref_lvalueContext*>(ctx->parent->parent);
	BashppParser::Ref_rvalueContext* parent_of_parent_rvalue = dynamic_cast<BashppParser::Ref_rvalueContext*>(ctx->parent->parent);
	if (parent_of_parent_lvalue != nullptr) {
		has_brace = (parent_of_parent_lvalue->LBRACE() != nullptr) || (parent_of_parent_lvalue->LBRACE_ROOTLEVEL() != nullptr);
	} else if (parent_of_parent_rvalue != nullptr) {
		has_brace = (parent_of_parent_rvalue->LBRACE() != nullptr) || (parent_of_parent_rvalue->LBRACE_ROOTLEVEL() != nullptr);
	} else {
		throw internal_error("Array index context has no parent", ctx);
	}
	if (has_brace) {
		object_reference_entity->set_array_index(array_index_entity->get_code());
		object_reference_entity->add_code_to_previous_line(array_index_entity->get_pre_code());
		object_reference_entity->add_code_to_next_line(array_index_entity->get_post_code());
		return;
	}
}
