/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_syntax_errors

	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::make_shared<bpp::bpp_object_assignment>();
	object_assignment->set_containing_class(entity_stack.top()->get_containing_class());
	object_assignment->inherit(latest_code_entity());
	entity_stack.push(object_assignment);
}

void BashppListener::exitObject_assignment(BashppParser::Object_assignmentContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
	entity_stack.pop();

	if (object_assignment == nullptr) {
		throw internal_error("Object assignment context was not found in the entity stack", ctx);
	}

	bool is_nonprimitive_copy = object_assignment->lvalue_is_nonprimitive() && object_assignment->rvalue_is_nonprimitive();

	if (is_nonprimitive_copy) {
		// We need to call the __copy method on the object
		// Verify that they're both of the same class
		std::shared_ptr<bpp::bpp_entity> lvalue_object = object_assignment->get_lvalue_object();
		std::shared_ptr<bpp::bpp_entity> rvalue_object = object_assignment->get_rvalue_object();
		if (lvalue_object == nullptr || rvalue_object == nullptr) {
			throw internal_error("Objects are null", ctx);
		}

		if (lvalue_object->get_class() == nullptr || rvalue_object->get_class() == nullptr) {
			throw internal_error("Objects have no class", ctx);
		}

		if (lvalue_object->get_class()->get_name() != rvalue_object->get_class()->get_name()) {
			throw_syntax_error_from_exitRule_ctx(ctx, "Cannot copy objects of different classes");
		}

		// Call the __copy method
		// Form: bpp__CLASSNAME____copy copyFromAddress copyToAddress copyFromIsPointer copyToIsPointer
		// Since we'll always pass pointers, the last two arguments will always be 1 here
		std::string method_call = "bpp__" + lvalue_object->get_class()->get_name() + "____copy ";

		// Get the addresses of the objects
		method_call += object_assignment->get_rvalue() + " " + object_assignment->get_lvalue();

		std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
		if (current_code_entity != nullptr) {
			current_code_entity->add_code_to_previous_line(object_assignment->get_pre_code());
			current_code_entity->add_code_to_next_line(object_assignment->get_post_code());
			current_code_entity->add_code(method_call);
			return;
		}
	}

	if (object_assignment->lvalue_is_nonprimitive() && !object_assignment->rvalue_is_nonprimitive()) {
		throw_syntax_error_from_exitRule_ctx(ctx->value_assignment(), "Cannot assign a primitive value to a nonprimitive object");
	}

	std::string object_assignment_lvalue = object_assignment->get_lvalue();
	std::string object_assignment_rvalue = object_assignment->get_rvalue();
	std::string pre_objectassignment_code = object_assignment->get_pre_code();
	std::string post_objectassignment_code = object_assignment->get_post_code();

	std::string assignment_variable_name = "____assignment" + std::to_string(program->get_assignment_counter());
	program->increment_assignment_counter();

	pre_objectassignment_code += assignment_variable_name + "=" + object_assignment_rvalue + "\n";
	post_objectassignment_code += "unset " + assignment_variable_name + "\n";

	std::string assignment_operator = object_assignment->is_adding() ? "+=" : "=";

	std::string object_assignment_code;

	if (object_assignment->rvalue_is_array()) {
		object_assignment_code = "eval \"" + object_assignment_lvalue + assignment_operator + "(\\\"\\${" + assignment_variable_name + "[@]}\\\")\"\n";
	} else {
		object_assignment_code = "eval " + object_assignment_lvalue + assignment_operator + "\\$" + assignment_variable_name + "\n";
	}

	// If we're not in a broader context, simply add the object assignment code to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(pre_objectassignment_code);
		current_code_entity->add_code_to_next_line(post_objectassignment_code);
		current_code_entity->add_code(object_assignment_code);
		return;
	}
}
