/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_
#define SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Object references take the form
	 * 	@IDENTIFIER.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either an object or a method
	 * If it's a primitive object, replace the reference with the address of the primitive object
	 * If it's a non-primitive object, this is a method call to .toPrimitive
	 * If it's a method, replace the reference with a call to the method
	 * There is no need to call the method in a supershell for lvalues. We can (and must) just call it directly
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER_LVALUE(), "Object reference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_class> current_class = current_code_entity->get_containing_class().lock();
	
	// Are we in an object_assignment context?
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());

	std::shared_ptr<bpp::bpp_string> object_reference_entity = std::make_shared<bpp::bpp_string>();
	object_reference_entity->set_containing_class(current_code_entity->get_containing_class());
	object_reference_entity->inherit(current_code_entity);
	entity_stack.push(object_reference_entity);

	std::shared_ptr<bpp::bpp_object> referenced_object = current_code_entity->get_object(ctx->IDENTIFIER_LVALUE()->getText());
	if (referenced_object == nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->AT(), "Object not found: " + ctx->IDENTIFIER_LVALUE()->getText());
	}

	std::vector<std::shared_ptr<bpp::bpp_entity>> object_chain;
	object_chain.push_back(referenced_object);

	std::shared_ptr<bpp::bpp_entity> current_context = referenced_object;

	bool can_descend = true;

	for (size_t i = 0; i < ctx->IDENTIFIER().size(); i++) {
		if (!can_descend) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(i), "Cannot descend further");
		}

		bool is_datamember = false;
		bool is_method = false;

		std::shared_ptr<bpp::bpp_datamember> referenced_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(current_context->get_class()->get_datamember(ctx->IDENTIFIER(i)->getText(), current_class));
		std::shared_ptr<bpp::bpp_method> referenced_method = std::dynamic_pointer_cast<bpp::bpp_method>(current_context->get_class()->get_method(ctx->IDENTIFIER(i)->getText(), current_class));

		if (referenced_datamember == bpp::inaccessible_datamember || referenced_method == bpp::inaccessible_method) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(i), ctx->IDENTIFIER(i)->getText() + " is inaccessible in this context");
		}

		if (referenced_datamember != nullptr) {
			is_datamember = true;
			object_chain.push_back(referenced_datamember);
			current_context = referenced_datamember;
		} else if (referenced_method != nullptr) {
			is_method = true;
			can_descend = false;
			object_chain.push_back(referenced_method);
		} else {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(i), "Member not found: " + ctx->IDENTIFIER(i)->getText());
		}
	}

	// Check the type of the last element in the object chain
	std::shared_ptr<bpp::bpp_object> final_object = std::dynamic_pointer_cast<bpp::bpp_object>(object_chain[object_chain.size() - 1]);
	std::shared_ptr<bpp::bpp_method> final_method = std::dynamic_pointer_cast<bpp::bpp_method>(object_chain[object_chain.size() - 1]);

	if (final_object != nullptr && final_object->get_class() != primitive && !final_object->is_pointer()) {
		if (object_assignment != nullptr) {
			object_assignment->set_lvalue_nonprimitive(true);
			object_assignment->set_lvalue_object(final_object);
		} else {
			// Call to .toPrimitive on a non-primitive data member
			// Add the toPrimitive method to the object_chain
			// Set final_method = toPrimitive
			// Set final_datamember = nullptr
			final_method = final_object->get_class()->get_method("toPrimitive", current_class);
			object_chain.push_back(final_method);
			final_object = nullptr;
		}
	}

	bool declared_first_temporary_variable = false;

	for (size_t i = 2; i < object_chain.size() - 1; i++) {
		std::string indirection = declared_first_temporary_variable ? "!" : "";
		std::string temporary_variable_lvalue;
		std::string temporary_variable_rvalue;
		std::string temporary_variable;
		if (referenced_object->is_pointer()) {
			temporary_variable = "bpp____ptr__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();
		} else {
			temporary_variable = "bpp__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();
		}
		for (size_t j = 1; j < i; j++) {
			temporary_variable += "__" + object_chain[j]->get_name();
		}
		temporary_variable_lvalue = temporary_variable + "__" + object_chain[i]->get_name();
		temporary_variable_rvalue = "${" + indirection + temporary_variable + "}__" + object_chain[i]->get_name();
		object_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
		object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
		declared_first_temporary_variable = true;
	}
	/**
	 * By this point, the deepest reference in the chain that we have access to is:
	 * ${bpp__objectClass__object1__object2__object3__....objectN-1}
	 * Where objectN (the next one, the one we don't have yet) is either a primitive or a method
	 */

	if (final_object != nullptr) {
		std::string indirection = declared_first_temporary_variable ? "!" : "";
		std::string temporary_variable_lvalue;
		std::string temporary_variable_rvalue;
		std::string temporary_variable;
		if (referenced_object->is_pointer()) {
			temporary_variable = "bpp____ptr__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();
		} else {
			temporary_variable = "bpp__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();
		}
		for (size_t i = 1; i < object_chain.size() - 1; i++) {
			temporary_variable += "__" + object_chain[i]->get_name();
		}
		// TODO(@rail5): Below: super hacky! Redo this whole thing from scratch ASAP
		if (object_assignment != nullptr && final_object->get_class() != primitive && !final_object->is_pointer()) {
			switch (object_chain.size()) {
			case 1:
				temporary_variable_lvalue = temporary_variable;
				break;
			case 2:
				temporary_variable_lvalue = "${" + temporary_variable + "__" + final_object->get_name() + "}";
				break;
			default:
				temporary_variable_lvalue = temporary_variable + "__" + final_object->get_name();
				break;
			}
		// This just gets worse and worse. Redo the whole object_reference_as_lvalue ASAP
		} else if (object_assignment != nullptr && final_object->get_class() != primitive && final_object->is_pointer() && object_chain.size() == 1) {
			temporary_variable_lvalue = temporary_variable;
		} else if (object_assignment != nullptr && final_object->get_class() != primitive && final_object->is_pointer() && object_chain.size() == 2) {
			temporary_variable_lvalue = "${" + temporary_variable + "}__" + final_object->get_name();
		} else {
			temporary_variable_lvalue = temporary_variable + "__" + final_object->get_name();
		}
		temporary_variable_rvalue = "${" + indirection + temporary_variable + "}__" + final_object->get_name();

		if (object_chain.size() > 1) {
			object_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
			object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
		}

		std::string encasing_start = "";
		std::string encasing_end = "";

		if (object_chain.size() > 1) {
			encasing_start = "${";
			if (object_assignment != nullptr && final_object->get_class() != primitive && !final_object->is_pointer()) {
				encasing_start += "!";
			}
			encasing_end = "}";
		}

		object_reference_entity->add_code(encasing_start + temporary_variable_lvalue + encasing_end);
	} else if (final_method != nullptr) {
		// Call the given method

		// Get the penultimate object in the chain
		std::shared_ptr<bpp::bpp_entity> penultimate_object = object_chain[object_chain.size() - 2];
		if (penultimate_object == nullptr) {
			throw internal_error("Penultimate entity in the object chain is not an object");
		}
		// Get its class
		std::shared_ptr<bpp::bpp_class> penultimate_class = penultimate_object->get_class();
		if (penultimate_class == nullptr) {
			throw internal_error("Penultimate entity in the object chain does not have a class");
		}

		std::string object_address = "bpp__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();

		for (size_t i = 1; i < object_chain.size() - 1; i++) {
			object_address += "__" + object_chain[i]->get_name();
		}

		std::string indirection_start = "";
		std::string indirection_end = "";

		if (object_chain.size() > 2) {
			indirection_start = "${";
			indirection_end = "}";
		}

		if (object_chain.size() > 3) {
			indirection_start += "!";
		}
		
		std::string method_call = "bpp__" + penultimate_class->get_name() + "__" + final_method->get_name();
		object_reference_entity->add_code(method_call + " \"" + indirection_start + object_address + indirection_end +"\" 1");

		std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
		if (object_assignment != nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER().back(), "Cannot assign to a method");
		}

	} else {
		throw internal_error("Terminal entity in the object chain is neither an object nor a method");
	}

}

void BashppListener::exitObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	entity_stack.pop();
	if (object_reference_entity == nullptr) {
		throw internal_error("Object reference context was not found in the entity stack");
	}

	// If we're in an object assignment, set the lvalue to the object reference code
	std::shared_ptr<bpp::bpp_object_assignment> current_object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
	if (current_object_assignment != nullptr) {
		current_object_assignment->set_lvalue(object_reference_entity->get_code());
		current_object_assignment->add_code_to_previous_line(object_reference_entity->get_pre_code());
		current_object_assignment->add_code_to_next_line(object_reference_entity->get_post_code());
		return;
	}

	// If we're not in a broader context, simply add the current object access code to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(object_reference_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(object_reference_entity->get_post_code());
		current_code_entity->add_code(object_reference_entity->get_code());
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_
