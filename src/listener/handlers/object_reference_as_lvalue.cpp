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
	 * Lvalue object references take the form
	 * 	@IDENTIFIER_LVALUE.IDENTIFIER.IDENTIFIER...
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
		throw_syntax_error(ctx->AT(), "Object reference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_class> current_class = current_code_entity->get_containing_class().lock();

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	object_reference_entity->set_containing_class(current_code_entity->get_containing_class());
	object_reference_entity->inherit(current_code_entity);
	entity_stack.push(object_reference_entity);

	// Start by getting the first identifier
	std::string first_object_name = ctx->IDENTIFIER_LVALUE()->getText();
	std::shared_ptr<bpp::bpp_object> first_object = current_code_entity->get_object(first_object_name);
	if (first_object == nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->AT(), "Object not found: " + first_object_name);
	}

	bpp::reference_type last_reference_type = bpp::reference_type::ref_object;
	std::shared_ptr<bpp::bpp_entity> last_reference_entity = first_object;

	// If it's a method, which class contains the method?
	// This shared_ptr will be filled in & updated as we go along
	std::shared_ptr<bpp::bpp_class> class_containing_the_method;

	std::shared_ptr<bpp::bpp_object> object = first_object;
	std::shared_ptr<bpp::bpp_datamember> datamember;
	std::shared_ptr<bpp::bpp_method> method;

	bool created_first_temporary_variable = false;
	bool created_second_temporary_variable = false;
	std::string encase_open = "";
	std::string encase_close = "";
	std::string indirection = "";

	// Generate the first bit of pre-code
	std::string object_reference_code;
	if (first_object->is_pointer()) {
		created_first_temporary_variable = true;
		encase_open = "${";
		encase_close = "}";
		object_reference_code = first_object->get_address();
	} else {
		object_reference_code = "bpp__" + first_object->get_class()->get_name() + "__" + first_object->get_name();
	}

	// Iterate over the list of identifiers
	for (auto& identifier : ctx->IDENTIFIER()) {
		bool throw_error = false;
		std::string error_string = "";
		switch (last_reference_type) {
			case bpp::reference_type::ref_object:
				break;
			case bpp::reference_type::ref_primitive:
				throw_error = true;
				error_string = "Unexpected identifier after primitive object reference";
				break;
			case bpp::reference_type::ref_method:
				throw_error = true;
				error_string = "Unexpected identifier after method reference";
				break;
			default:
				throw internal_error("Unknown reference type");
		}

		if (throw_error) {
			entity_stack.pop();
			throw_syntax_error(identifier, error_string);
		}

		std::string identifier_text = identifier->getText();

		std::shared_ptr<bpp::bpp_class> reference_class = last_reference_entity->get_class();

		object = nullptr;
		datamember = reference_class->get_datamember(identifier_text, current_class);
		method = reference_class->get_method(identifier_text, current_class);

		if (datamember == bpp::inaccessible_datamember || method == bpp::inaccessible_method) {
			entity_stack.pop();
			throw_syntax_error(identifier, identifier_text + " is inaccessible in this context");
		}

		if (method != nullptr) {
			class_containing_the_method = last_reference_entity->get_class();
			last_reference_type = bpp::reference_type::ref_method;
			last_reference_entity = method;
		} else if (datamember != nullptr) {
			last_reference_type = (datamember->get_class() == primitive) ? bpp::reference_type::ref_primitive : bpp::reference_type::ref_object;
			last_reference_entity = datamember;

			indirection = created_second_temporary_variable ? "!" : "";
			std::string temporary_variable_lvalue = object_reference_code + "__" + identifier_text;
			std::string temporary_variable_rvalue = "${" + indirection + object_reference_code + "}__" + identifier_text;

			if (created_first_temporary_variable) {
				object_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
				object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
				created_second_temporary_variable = true;
			}

			object_reference_code = temporary_variable_lvalue;
			created_first_temporary_variable = true;
		} else {
			entity_stack.pop();
			throw_syntax_error(identifier, last_reference_entity->get_name() + " has no member named " + identifier_text);
		}
	}

	object_reference_entity->set_reference_type(last_reference_type);

	// Are we in an object_assignment context?
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(current_code_entity);

	// Now that we've finished iterating through the identifiers
	encase_open = created_first_temporary_variable ? "${" : "";
	encase_close = created_first_temporary_variable ? "}" : "";
	indirection = created_second_temporary_variable ? "!" : "";

	if (object_assignment != nullptr) {
		// Special encasing rules for object assignments
		encase_open = created_second_temporary_variable ? "${" : "";
		encase_close = created_second_temporary_variable ? "}" : "";
		indirection = "";
	}

	if (last_reference_type == bpp::reference_type::ref_method) {
		if (object_assignment != nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->AT(), "Cannot assign to a method");
		}
		// Call the method directly -- not in a supershell
		std::string method_call = "bpp__" + class_containing_the_method->get_name() + "__" + method->get_name() + " ";
		method_call += encase_open + indirection + object_reference_code + encase_close;
		method_call += " 1";

		object_reference_entity->add_code(method_call);
		return;
	}

	// Are we accessing an index of an array?
	if (ctx->array_index() != nullptr) {
		std::string counting = ctx->POUNDKEY() != nullptr ? "#" : "";

		bool have_to_dereference_a_pointer = ctx->IDENTIFIER().size() > 1;

		std::string temporary_variable_lvalue = object_reference_code + "____arrayIndexString";
		std::string temporary_variable_rvalue;

		if (have_to_dereference_a_pointer) {
			temporary_variable_rvalue = counting + "${" + object_reference_code + "}[" + object_reference_entity->get_array_index() + "]";
		} else {
			temporary_variable_rvalue = "${" + counting + indirection + object_reference_code + "[" + object_reference_entity->get_array_index() + "]}";
		}

		object_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
		object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

		temporary_variable_rvalue = "${" + object_reference_code + "____arrayIndexString}";

		if (ctx->POUNDKEY() != nullptr) {
			temporary_variable_rvalue = "\\${" + temporary_variable_rvalue + "}";
		}

		if (have_to_dereference_a_pointer) {
			temporary_variable_lvalue = object_reference_code + "____arrayIndex";
			object_reference_entity->add_code_to_previous_line("eval " + temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
			object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
		}

		object_reference_code = temporary_variable_lvalue;
	}

	bool datamember_is_pointer = (datamember != nullptr) && (datamember->is_pointer());
	std::shared_ptr<bpp::bpp_object> last_reference_object = std::dynamic_pointer_cast<bpp::bpp_object>(last_reference_entity);
	std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(current_code_entity);

	if (last_reference_type == bpp::reference_type::ref_primitive || datamember_is_pointer) {
		if (ctx->POUNDKEY() != nullptr) {
			indirection = "";
		}
		object_reference_entity->add_code(encase_open + indirection + object_reference_code + encase_close);

		if (delete_entity != nullptr && datamember_is_pointer) {

			delete_entity->set_object_to_delete(last_reference_object);
			delete_entity->set_force_pointer(true);
		}
		return;
	}

	if (last_reference_entity->get_class() == nullptr) {
		throw internal_error("Last reference entity has no class");
	}

	// Are we dereferencing a pointer?
	std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference = std::dynamic_pointer_cast<bpp::bpp_pointer_dereference>(current_code_entity);

	// If we're here, the last reference entity is a non-primitive object
	// Is it a pointer?
	if (last_reference_object != nullptr && last_reference_object->is_pointer()) {
		if (pointer_dereference != nullptr) {
			if (object_assignment != nullptr) {
				object_assignment->set_lvalue_nonprimitive(true);
				object_assignment->set_lvalue_object(last_reference_object);
				object_reference_entity->add_code(encase_open + indirection + object_reference_code + encase_close);
			} else {
				// Call .toPrimitive
				std::string method_call = "bpp__" + last_reference_object->get_class()->get_name() + "__toPrimitive ";
				method_call += "${" + object_reference_code + "} 1";

				object_reference_entity->add_code(method_call);
			}
			return;
		} else {
			object_reference_entity->add_code(encase_open + indirection + object_reference_code + encase_close);
			return;
		}
	}

	if (object_assignment != nullptr) {
		if (last_reference_object != nullptr && (!last_reference_object->is_pointer() || pointer_dereference != nullptr)) {
			object_assignment->set_lvalue_nonprimitive(true);
			object_assignment->set_lvalue_object(last_reference_object);
		}
		object_reference_entity->add_code(encase_open + indirection + object_reference_code + encase_close);
		return;
	}

	if (last_reference_object != nullptr) {
		// Call .toPrimitive
		std::string method_call = "bpp__" + last_reference_object->get_class()->get_name() + "__toPrimitive ";
		method_call += encase_open + indirection + object_reference_code + encase_close + " 1";
		object_reference_entity->add_code(method_call);
	}
}

void BashppListener::exitObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());

	if (object_reference_entity == nullptr) {
		throw internal_error("Object reference entity not found on the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found on the entity stack");
	}

	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(current_code_entity);

	// Are we in an object assignment?
	if (object_assignment != nullptr) {
		object_assignment->set_lvalue(object_reference_entity->get_code());
		object_assignment->add_code_to_previous_line(object_reference_entity->get_pre_code());
		object_assignment->add_code_to_next_line(object_reference_entity->get_post_code());
		return;
	}

	// If we're not in any broader context, simply add the object reference to the current code entity
	current_code_entity->add_code_to_previous_line(object_reference_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(object_reference_entity->get_post_code());
	current_code_entity->add_code(object_reference_entity->get_code());
}

#endif // SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_AS_LVALUE_CPP_
