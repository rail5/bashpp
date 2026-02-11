/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterValueAssignment(std::shared_ptr<AST::ValueAssignment> node) {
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::make_shared<bpp::bpp_value_assignment>();
	value_assignment_entity->set_containing_class(entity_stack.top()->get_containing_class());
	value_assignment_entity->inherit(latest_code_entity());

	// If we're in an object assignment context, determine whether the lvalue is primitive or nonprimitive
	bool lvalue_nonprimitive = false;
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
	if (object_assignment != nullptr) {
		value_assignment_entity->set_lvalue_nonprimitive(object_assignment->lvalue_is_nonprimitive());
		lvalue_nonprimitive = object_assignment->lvalue_is_nonprimitive();
	}

	// If this value assignment is attached to an object instantiation, the lvalue is of course nonprimitive
	std::shared_ptr<bpp::bpp_object> object_instantiation = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
	if (object_instantiation != nullptr) {
		value_assignment_entity->set_lvalue_nonprimitive(true);
		lvalue_nonprimitive = true;
	}
	
	auto op = node->OPERATOR();
	value_assignment_entity->set_adding(op.getValue() == "+=");

	entity_stack.push(value_assignment_entity);
	if (lvalue_nonprimitive) {
		context_expectations_stack.push({false, true}); // Nonprimitive lvalue: rvalue must be nonprimitive
	} else {
		context_expectations_stack.push({true, false}); // Primitive lvalue: rvalue must be primitive
	}
}

void BashppListener::exitValueAssignment(std::shared_ptr<AST::ValueAssignment> node) {
	/**
	 * Value assignments will appear in the following contexts:
	 * 	1. Member declarations
	 * 	2. Object instantiations
	 * 	3. Pointer declarations
	 * 	4. Object assignments
	 */

	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());
	entity_stack.pop();
	context_expectations_stack.pop();

	if (value_assignment_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Value assignment context was not found in the entity stack");
	}

	// Check if we're in a member declaration
	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (current_datamember != nullptr) {
		current_datamember->set_default_value(value_assignment_entity->get_code());
		current_datamember->set_pre_access_code(value_assignment_entity->get_pre_code());
		current_datamember->set_post_access_code(value_assignment_entity->get_post_code());

		// Check if this is an array assignment
		if (value_assignment_entity->is_array_assignment()) {
			current_datamember->set_array(true);
		}
		return;
	}

	// Check if we're in an object assignment
	std::shared_ptr<bpp::bpp_object_assignment> current_object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
	if (current_object_assignment != nullptr) {
		current_object_assignment->set_adding(value_assignment_entity->is_adding());
		current_object_assignment->set_rvalue_array(value_assignment_entity->is_array_assignment());
		current_object_assignment->set_rvalue(value_assignment_entity->get_code());
		current_object_assignment->add_code_to_previous_line(value_assignment_entity->get_pre_code());
		current_object_assignment->add_code_to_next_line(value_assignment_entity->get_post_code());

		// Check if the rvalue is a nonprimitive object
		if (value_assignment_entity->is_nonprimitive_assignment()) {
			current_object_assignment->set_rvalue_nonprimitive(true);
			current_object_assignment->set_rvalue_object(value_assignment_entity->get_nonprimitive_object());
			std::string rvalue;
			std::shared_ptr<bpp::bpp_object> rvalue_object = std::dynamic_pointer_cast<bpp::bpp_object>(value_assignment_entity->get_nonprimitive_object());
			if (rvalue_object == nullptr) {
				throw bpp::ErrorHandling::InternalError("Rvalue object not found for copy");
			}
		}
		return;
	}

	// Check if we're in an object instantiation or pointer declaration
	std::shared_ptr<bpp::bpp_object> current_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
	if (current_object != nullptr) {
		// Is it a pointer?
		if (current_object->is_pointer()) {
			current_object->set_pre_access_code(value_assignment_entity->get_pre_code());
			current_object->set_post_access_code(value_assignment_entity->get_post_code());
			current_object->set_assignment_value(value_assignment_entity->get_code());
		} else if (!value_assignment_entity->is_nonprimitive_assignment()) {
			// The object we're assigning to is not a pointer, and yet we're trying to assign a primitive to it
			// Throw an error
			throw bpp::ErrorHandling::SyntaxError(this, node, "Cannot assign a primitive value to a nonprimitive object");
		}
		return;
	}

	// Default case: just send it up the chain
	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(value_assignment_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(value_assignment_entity->get_post_code());
	current_code_entity->add_code(node->OPERATOR().getValue() + value_assignment_entity->get_code());
}
