/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_CPP_
#define SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_reference(BashppParser::Object_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Object references take the form
	 * 	@IDENTIFIER.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either an object or a method
	 * If it's a primitive object, treat this as an rvalue and get the value of the primitive object
	 * If it's a non-primitive object, this is a method call to .toPrimitive
	 * If it's a method, call the method in a supershell and substitute the result
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(0), "Object reference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_class> current_class = current_code_entity->get_containing_class().lock();

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	object_reference_entity->set_containing_class(current_code_entity->get_containing_class());
	object_reference_entity->inherit(current_code_entity);
	entity_stack.push(object_reference_entity);
}

void BashppListener::exitObject_reference(BashppParser::Object_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	BashppParser::Ref_rvalueContext* parent = dynamic_cast<BashppParser::Ref_rvalueContext*>(ctx->parent);
	bool hasPoundKey = parent->POUNDKEY() != nullptr;

	// Get the object reference entity
	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());
	entity_stack.pop();
	if (object_reference_entity == nullptr) {
		throw internal_error("Object reference context was not found in the entity stack", ctx);
	}

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error_from_exitRule(ctx->IDENTIFIER(0), "Object reference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_class> current_class = current_code_entity->get_containing_class().lock();

	// Check if we're in an object_address context
	// This will be important later -- we'll have to return differently if we are
	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::dynamic_pointer_cast<bpp::bpp_object_address>(entity_stack.top());

	// Check if we're in a value_assignment context
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());

	// Are we dereferencing a pointer?
	std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference = std::dynamic_pointer_cast<bpp::bpp_pointer_dereference>(current_code_entity);

	bpp::reference_type last_reference_type = bpp::reference_type::ref_object;
	std::shared_ptr<bpp::bpp_entity> last_reference_entity = current_code_entity;

	std::shared_ptr<bpp::bpp_object> object = nullptr;
	bool object_is_pointer = false;
	bool first_object_is_pointer = false;
	std::shared_ptr<bpp::bpp_datamember> datamember = nullptr;
	bool datamember_is_pointer = false;
	std::shared_ptr<bpp::bpp_method> method = nullptr;
	std::shared_ptr<bpp::bpp_class> class_containing_the_method = nullptr;

	bool created_first_temporary_variable = false;
	bool created_second_temporary_variable = false;
	std::string indirection = "";

	std::string object_reference_code = "";

	std::string encase_open = ctx->IDENTIFIER().size() > 2 ? "${" : "";
	std::string encase_close = ctx->IDENTIFIER().size() > 2 ? "}" : "";

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
				throw internal_error("Unknown reference type", ctx);
		}

		if (throw_error) {
			throw_syntax_error_from_exitRule(identifier, error_string);
		}
		std::string identifier_text = identifier->getText();

		std::shared_ptr<bpp::bpp_class> reference_class = last_reference_entity->get_class();
		if (reference_class != nullptr) {
			object = nullptr;
			datamember = reference_class->get_datamember(identifier_text, current_class);
			method = reference_class->get_method(identifier_text, current_class);
		} else {
			object = last_reference_entity->get_object(identifier_text);
			first_object_is_pointer = (object == nullptr) ? false : object->is_pointer();
		}

		if (reference_class == nullptr && object == nullptr) {
			throw_syntax_error_from_exitRule(identifier, "Object not found: " + identifier_text);
		}

		if (datamember == bpp::inaccessible_datamember || method == bpp::inaccessible_method) {
			throw_syntax_error_from_exitRule(identifier, identifier_text + " is inaccessible in this context");
		}

		if (object != nullptr) {
			last_reference_type = bpp::reference_type::ref_object;
			last_reference_entity = object;
			object_is_pointer = object->is_pointer();
			if (!object_is_pointer) {
				object_reference_code = "bpp__" + object->get_class()->get_name() + "__" + object->get_name();
			} else {
				object_reference_code = object->get_address();
				created_first_temporary_variable = true;
				encase_open = "${";
				encase_close = "}";
			}
		} else if (method != nullptr) {
			class_containing_the_method = last_reference_entity->get_class();
			last_reference_type = bpp::reference_type::ref_method;
			last_reference_entity = method;
		} else if (datamember != nullptr) {
			bool is_primitive = datamember->get_class() == primitive;
			datamember_is_pointer = datamember->is_pointer();
			last_reference_type = is_primitive ? bpp::reference_type::ref_primitive : bpp::reference_type::ref_object;
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
			throw_syntax_error_from_exitRule(identifier, last_reference_entity->get_name() + " has no member named " + identifier_text);
		}
	}

	object_reference_entity->set_reference_type(last_reference_type);

	bool ready_to_exit = false;

	if (last_reference_type == bpp::reference_type::ref_method) {
		indirection = ctx->IDENTIFIER().size() > 3 ? "!" : "";
		code_segment method_call_code = generate_method_call_code(encase_open + indirection + object_reference_code + encase_close, method->get_name(), class_containing_the_method);

		code_segment method_code = generate_supershell_code(method_call_code.pre_code + "\n" + method_call_code.code + "\n" + method_call_code.post_code);
		object_reference_entity->add_code_to_previous_line(method_code.pre_code);
		object_reference_entity->add_code_to_next_line(method_code.post_code);
		object_reference_entity->add_code(method_code.code);
		ready_to_exit = true;
	}

	if (!ready_to_exit) {
		// Are we accessing an index of an array?
		if (ctx->array_index() != nullptr) {
			// We're accessing an array index

			std::string counting = hasPoundKey ? "#" : "";

			/**
			 * Suppose we have an 'object' with a datamember 'array' which is an array of primitives
			 * Accessing index 'i' of that array would, in Bash++, take the form:
			 * 		@object.array[$i]
			 * And the compiled code's version of the reference:
			 * 		${bpp__objectClass__object__array[$i]}
			 * 
			 * Suppose however that 'object' has a non-primitive datamember 'inner' which has a datamember 'array' which is an array of primitives
			 * (In this case, we're nesting -- the array is not a datamember of 'object', but of 'object.inner')
			 * Accessing index 'i' of that array would, in Bash++, take the form:
			 * 		@object.inner.array[$i]
			 * But the compiled code's version of the reference would have to take a slightly different form,
			 * Since we have to dereference the pointer to 'inner' before we can access 'array':
			 * 		bpp__objectClass__object__inner__array=${bpp__objectClass__object__inner}__array
			 * This "bpp__objectClass__object__inner__array" evaluates to a STRING which is the variable name where the array is actually stored
			 * We then have to evaluate that string to get the actual values stored in the array:
			 * 		bpp__objectClass__object__inner__arrayString="${bpp__objectClass__object__inner__array}[${i}]"
			 * 			# This first line gives us a string such as "actual_array_variable_name[$i]"
			 * 			# This string is not a variable reference, but a string which represents the variable reference
			 * 		eval bpp__objectClass__object__inner__arrayIndex="\${${bpp__objectClass__object__inner__arrayString}"}"
			 * 			# This second line gives us the actual value stored in the array at index 'i'
			 * 
			 * So, we have to follow a different procedure based on whether or not we have to dereference a pointer
			 * 
			 * To make this code less disgraceful, at some point, object references should be re-worked altogether
			 * In the meantime, we can do a HACKY fix by checking the size of the IDENTIFIER list
			 * If it's greater than 2, we're guaranteed to have to dereference a pointer
			 * If it's not, we're guaranteed not to have to dereference a pointer (unless the very first identifier refers to a pointer)
			 * 
			 * TODO(@rail5): Fix this. Really just fix object references from the ground-up
			 */

			bool have_to_dereference_a_pointer = first_object_is_pointer || (ctx->IDENTIFIER().size() > 2);

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

			// If we're counting, we need to add another small layer of abstraction
			// Ordinarily, by this point, we've set up a temporary variable whose value is a STRING (not in fact a variable reference,
			// but a string which represents the variable reference -- ie, rather than object[$value], a string such as "object[\$value]")
			// In the case that we're counting, that string has been modified to be "#object[\$value]",
			// And so we need to set the temporary variable to be the result of evaluating that string -- but only after that string is
			// Surrounded by ${} to make it a variable reference
			if (hasPoundKey) {
				temporary_variable_rvalue = "\\${" + temporary_variable_rvalue + "}";
			}

			if (have_to_dereference_a_pointer) {
				temporary_variable_lvalue = object_reference_code + "____arrayIndex";
				object_reference_entity->add_code_to_previous_line("eval " + temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
				object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			}

			object_reference_code = temporary_variable_lvalue;
		}

		if (last_reference_type == bpp::reference_type::ref_primitive || (datamember_is_pointer && pointer_dereference == nullptr)) {
			indirection = (created_second_temporary_variable && !hasPoundKey) ? "!" : "";
			object_reference_entity->add_code("${" + indirection + object_reference_code + "}");

			std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
			if (delete_entity != nullptr && datamember_is_pointer) {
				std::shared_ptr<bpp::bpp_object> last_reference_object = std::dynamic_pointer_cast<bpp::bpp_object>(last_reference_entity);
				delete_entity->set_object_to_delete(last_reference_object);
				delete_entity->set_force_pointer(true);
			}
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		if (last_reference_entity->get_class() == nullptr) {
			throw internal_error("Last reference entity has no class", ctx);
		}

		// If we're here, the last reference entity is a non-primitive object
		std::shared_ptr<bpp::bpp_object> last_reference_object = std::dynamic_pointer_cast<bpp::bpp_object>(last_reference_entity);
		std::string code_to_add = "";

		// Are we in a delete statement?
		std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
		if (delete_entity != nullptr) {
			delete_entity->set_object_to_delete(last_reference_object);
			delete_entity->set_force_pointer(true);
			code_to_add = object_reference_code;
			ready_to_exit = true;
		}

		// Is it a pointer?
		if (last_reference_object != nullptr && last_reference_object->is_pointer()) {
			// Are we dereferencing a pointer?
			if (pointer_dereference != nullptr) {
				// Is this pointer dereference part of a larger value assignment?
				if (pointer_dereference->get_value_assignment() != nullptr) {
					pointer_dereference->add_code_to_previous_line(object_reference_entity->get_pre_code());
					pointer_dereference->add_code_to_next_line(object_reference_entity->get_post_code());
					pointer_dereference->get_value_assignment()->set_nonprimitive_assignment(true);
					pointer_dereference->get_value_assignment()->set_nonprimitive_object(last_reference_object);
				} else {
					// Call .toPrimitive
					indirection = (created_second_temporary_variable && !hasPoundKey) ? "!" : "";

					code_segment method_call_code = generate_method_call_code("${" + indirection + object_reference_code + "}", "toPrimitive", last_reference_object->get_class());

					code_segment method_code = generate_supershell_code(method_call_code.pre_code + "\n" + method_call_code.code + "\n" + method_call_code.post_code);
					object_reference_entity->add_code_to_previous_line(method_code.pre_code);
					object_reference_entity->add_code_to_next_line(method_code.post_code);
					code_to_add = method_code.code;

					// Show a warning if we're doing a dynamic_cast
					std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(current_code_entity);
					if (dynamic_cast_entity != nullptr) {
						show_warning(ctx->IDENTIFIER(0), "Dynamic casting the result of .toPrimitive may not be what you want\nDid you mean to take the address of the object?");
					}
				}
				ready_to_exit = true;
			} else {
				indirection = created_second_temporary_variable ? "!" : "";
				code_to_add = "${" + indirection + object_reference_code + "}";
				ready_to_exit = true;
			}
		}

		object_reference_entity->add_code(code_to_add);
	}

	if (!ready_to_exit) {
		// Are we otherwise in an object_address context?
		if (object_address_entity != nullptr) {
			object_reference_entity->add_code("${" + object_reference_code + "}");
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		// Are we in a value_assignment context?
		if (value_assignment_entity != nullptr && value_assignment_entity->lvalue_is_nonprimitive()) {
			value_assignment_entity->set_nonprimitive_object(last_reference_entity);
			value_assignment_entity->set_nonprimitive_assignment(true);
			object_reference_entity->add_code(object_reference_code);
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		// We need to call the .toPrimitive method on the object
		encase_open = ctx->IDENTIFIER().size() > 1 ? "${" : "";
		encase_close = ctx->IDENTIFIER().size() > 1 ? "}" : "";
		indirection = ctx->IDENTIFIER().size() > 2 ? "!" : "";

		code_segment method_call_code = generate_method_call_code(encase_open + indirection + object_reference_code + encase_close, "toPrimitive", last_reference_entity->get_class());

		code_segment method_code = generate_supershell_code(method_call_code.pre_code + "\n" + method_call_code.code + "\n" + method_call_code.post_code);
		object_reference_entity->add_code_to_previous_line(method_code.pre_code);
		object_reference_entity->add_code_to_next_line(method_code.post_code);
		object_reference_entity->add_code(method_code.code);

		// Show a warning if we're doing a dynamic_cast
		std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(current_code_entity);
		if (dynamic_cast_entity != nullptr) {
			show_warning(ctx->IDENTIFIER(0), "Dynamic casting the result of .toPrimitive may not be what you want\nDid you mean to take the address of the object?");
		}
	}

	// Ready to exit

	// Are we in a delete statement?
	std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
	if (delete_entity != nullptr) {
		if (object_reference_entity->get_reference_type() == bpp::reference_type::ref_method) {
			throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot call @delete on a method");
		}

		if (object_reference_entity->get_reference_type() == bpp::reference_type::ref_primitive) {
			throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot call @delete on a primitive");
		}

		delete_entity->add_code_to_previous_line(object_reference_entity->get_pre_code());
		delete_entity->add_code_to_next_line(object_reference_entity->get_post_code());
		delete_entity->add_code(object_reference_entity->get_code());
		return;
	}

	// Are we in an object_address context?
	if (object_address_entity != nullptr) {
		if (object_reference_entity->get_reference_type() == bpp::reference_type::ref_method) {
			throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot get the address of a method");
		}
		std::string address = object_reference_entity->get_code();
		// Some hacky string manipulation to work backwards
		// If it starts with '${!', we'll change that to '${'
		// If it doesn't have the indirection exclamation point, but starts with '${', we'll remove the '${' and the closing '}'
		// TODO(@rail5): This is a hacky way to do this, and should be replaced with a more robust solution
		if (address.substr(0, 3) == "${!") {
			address = "${" + address.substr(3);
		} else if (address.substr(0, 2) == "${") {
			address = address.substr(2, address.size() - 3);
		}

		object_address_entity->add_code_to_previous_line(object_reference_entity->get_pre_code());
		object_address_entity->add_code_to_next_line(object_reference_entity->get_post_code());
		object_address_entity->add_code(address);
		return;
	}

	// If we're not in any broader context, simply add the object reference to the current code entity
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(object_reference_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(object_reference_entity->get_post_code());
		current_code_entity->add_code(object_reference_entity->get_code());
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_OBJECT_REFERENCE_CPP_
