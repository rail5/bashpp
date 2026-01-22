/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterDeleteStatement(std::shared_ptr<AST::DeleteStatement> node) {
	skip_syntax_errors
	/**
	 * Delete statements take the form
	 * 	@delete @object
	 * Where object is the name of the object to delete
	 * 
	 * This statement calls the __delete function for the object and the object's destructor if it exists
	 * If the destructor exists, it will be called *first* (before __delete)
	 * It then unsets the object
	 */

	// Get the current code entity

	// Create a new bpp_string for the delete statement
	std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::make_shared<bpp::bpp_delete_statement>();
	delete_entity->set_containing_class(entity_stack.top()->get_containing_class());
	delete_entity->inherit(latest_code_entity());
	entity_stack.push(delete_entity);
	context_expectations_stack.push({true, true}); // @delete can take both primitives and objects
}

void BashppListener::exitDeleteStatement(std::shared_ptr<AST::DeleteStatement> node) {
	skip_syntax_errors
	// Get the delete entity from the entity stack
	std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
	if (delete_entity == nullptr) {
		throw internal_error("Delete statement not found on the entity stack");
	}

	entity_stack.pop();
	context_expectations_stack.pop();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = latest_code_entity();

	if (delete_entity->get_object_to_delete() == nullptr) {
		auto objectReferenceEntity = std::dynamic_pointer_cast<AST::ObjectReference>(node->getFirstChild());

		if (!objectReferenceEntity) {
			throw internal_error("Delete statement does not contain a valid object reference");
		}

		std::string object_ref_name = "@" + objectReferenceEntity->IDENTIFIER().getValue();
		for (const auto& id : objectReferenceEntity->IDENTIFIERS()) {
			object_ref_name += "." + id.getValue();
		}

		throw_syntax_error_from_exitRule(node, "Object not found: " + object_ref_name);
	}

	if (delete_entity->get_object_to_delete()->get_address() == "${__this}") {
		throw_syntax_error_from_exitRule(node, "Cannot call @delete on '@this'");
	}

	// Generate the delete code
	code_segment delete_code = generate_delete_code(delete_entity->get_object_to_delete(), delete_entity->get_code(), program);

	// Add any necessary access code to the code entity
	code_entity->add_code_to_previous_line(delete_entity->get_pre_code());
	code_entity->add_code_to_next_line(delete_entity->get_post_code());

	// Add the delete code to the code entity
	code_entity->add_code_to_previous_line(delete_code.pre_code);
	code_entity->add_code_to_next_line(delete_code.post_code);

	code_entity->add_code(delete_code.code);
}
