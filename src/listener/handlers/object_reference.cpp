/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterObject_reference(BashppParser::Object_referenceContext *ctx) {
	skip_syntax_errors
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
	skip_syntax_errors
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

	std::deque<antlr4::tree::TerminalNode*> ids;
	for (auto& id : ctx->IDENTIFIER()) {
		ids.push_back(id);
	}

	std::shared_ptr<bpp::bpp_object> first_object = current_code_entity->get_object(ids.at(0)->getText());
	bool first_object_is_pointer = first_object != nullptr && first_object->is_pointer();

	bpp::entity_reference ref = bpp::resolve_reference(
		source_file,
		current_code_entity,
		&ids,
		in_class || in_supershell || in_bash_function,
		can_take_object(),
		can_take_primitive(),
		program
	);

	if (ref.error.has_value()) {
		throw_syntax_error_from_exitRule(ref.error->token, ref.error->message);
	}

	object_reference_entity->add_code_to_previous_line(ref.reference_code.pre_code);
	object_reference_entity->add_code_to_next_line(ref.reference_code.post_code);

	bpp::reference_type reference_type;
	std::string encase_open, encase_close, indirection;

	std::shared_ptr<bpp::bpp_object> object = std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity);
	std::shared_ptr<bpp::bpp_datamember> datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(ref.entity);
	bool datamember_is_pointer = datamember != nullptr && datamember->is_pointer();
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(ref.entity);

	if (method != nullptr) {
		reference_type = bpp::reference_type::ref_method;
	} else if (datamember != nullptr) {
		reference_type = datamember->get_class() == program->get_primitive_class()
			? bpp::reference_type::ref_primitive
			: bpp::reference_type::ref_object;
	} else if (object != nullptr) {
		reference_type = bpp::reference_type::ref_object;
	} else {
		throw internal_error("Referenced entity is not an object, data member or method", ctx);
	}

	object_reference_entity->set_reference_type(reference_type);

	encase_open = ref.created_first_temporary_variable ? "${" : "";
	encase_close = ref.created_first_temporary_variable ? "}" : "";
	indirection = ref.created_second_temporary_variable ? "!" : "";

	bool ready_to_exit = false;

	if (reference_type == bpp::reference_type::ref_method) {
		indirection = ctx->IDENTIFIER().size() > 3 ? "!" : "";
		code_segment method_call_code = generate_method_call_code(encase_open + indirection + ref.reference_code.code + encase_close, method->get_name(), ref.class_containing_the_method, false, program);
		// Are we taking the *address* of the method or are we calling it?
		if (object_address_entity != nullptr) {
			/**
			 * If we're taking the address of the method, return the method_call_code
			 * This will be in the format:
			 * 		{function_name} {object_pointer}
			 * E.g, if we have a method called 'foo' in class 'bar', and we're taking the address "&@object.foo"
			 * 		Say for example:
			 * 			echo &@object.foo
			 * Then it will return:
			 * 		bpp__bar__foo address__of__object
			 */
			object_reference_entity->add_code_to_previous_line(method_call_code.pre_code);
			object_reference_entity->add_code_to_next_line(method_call_code.post_code);
			object_reference_entity->add_code(method_call_code.code);
		} else {
			/**
			 * If we're not taking the address (most cases), then we need to run the method in a supershell
			 * And substitute the result of the supershell in place of the reference
			 */
			code_segment method_code = generate_supershell_code(method_call_code.full_code(), in_while_condition, current_while_condition, program);
			object_reference_entity->add_code_to_previous_line(method_code.pre_code);
			object_reference_entity->add_code_to_next_line(method_code.post_code);
			object_reference_entity->add_code(method_code.code);
		}
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

			std::string temporary_variable_lvalue = ref.reference_code.code + "____arrayIndexString";
			std::string temporary_variable_rvalue;

			if (have_to_dereference_a_pointer) {
				temporary_variable_rvalue = counting + "${" + ref.reference_code.code + "}[" + object_reference_entity->get_array_index() + "]";
			} else {
				temporary_variable_rvalue = "${" + counting + indirection + ref.reference_code.code + "[" + object_reference_entity->get_array_index() + "]}";
			}

			object_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

			temporary_variable_rvalue = "${" + ref.reference_code.code + "____arrayIndexString}";

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
				temporary_variable_lvalue = ref.reference_code.code + "____arrayIndex";
				object_reference_entity->add_code_to_previous_line("eval " + temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
				object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			}

			ref.reference_code.code = temporary_variable_lvalue;
		}

		if (reference_type == bpp::reference_type::ref_primitive || (datamember_is_pointer && pointer_dereference == nullptr)) {
			indirection = (ref.created_second_temporary_variable && !hasPoundKey) ? "!" : "";
			object_reference_entity->add_code("${" + indirection + ref.reference_code.code + "}");

			std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
			if (delete_entity != nullptr && datamember_is_pointer) {
				delete_entity->set_object_to_delete(object);
				delete_entity->set_force_pointer(true);
			}
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		if (ref.entity->get_class() == nullptr) {
			throw internal_error("Last reference entity has no class", ctx);
		}

		// If we're here, the last reference entity is a non-primitive object
		std::shared_ptr<bpp::bpp_object> last_reference_object = std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity);
		std::string code_to_add = "";

		// Are we in a delete statement?
		std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
		if (delete_entity != nullptr) {
			delete_entity->set_object_to_delete(last_reference_object);
			delete_entity->set_force_pointer(true);
			code_to_add = ref.reference_code.code;
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
					indirection = (ref.created_second_temporary_variable && !hasPoundKey) ? "!" : "";

					code_segment method_call_code = generate_method_call_code("${" + indirection + ref.reference_code.code + "}", "toPrimitive", last_reference_object->get_class(), false, program);

					code_segment method_code = generate_supershell_code(method_call_code.full_code(), in_while_condition, current_while_condition, program);
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
				indirection = ref.created_second_temporary_variable ? "!" : "";
				code_to_add = "${" + indirection + ref.reference_code.code + "}";
				ready_to_exit = true;
			}
		}

		object_reference_entity->add_code(code_to_add);
	}

	if (!ready_to_exit) {
		// Are we otherwise in an object_address context?
		if (object_address_entity != nullptr) {
			object_reference_entity->add_code("${" + ref.reference_code.code + "}");
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		// Are we in a value_assignment context?
		if (value_assignment_entity != nullptr && value_assignment_entity->lvalue_is_nonprimitive()) {
			value_assignment_entity->set_nonprimitive_object(ref.entity);
			value_assignment_entity->set_nonprimitive_assignment(true);
			object_reference_entity->add_code(ref.reference_code.code);
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		// We need to call the .toPrimitive method on the object
		encase_open = ctx->IDENTIFIER().size() > 1 ? "${" : "";
		encase_close = ctx->IDENTIFIER().size() > 1 ? "}" : "";
		indirection = ctx->IDENTIFIER().size() > 2 ? "!" : "";

		code_segment method_call_code = generate_method_call_code(encase_open + indirection + ref.reference_code.code + encase_close, "toPrimitive", ref.entity->get_class(), false, program);

		code_segment method_code = generate_supershell_code(method_call_code.full_code(), in_while_condition, current_while_condition, program);
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
		std::string address = object_reference_entity->get_code();
		// Some hacky string manipulation to work backwards
		// If it starts with '${!', we'll change that to '${'
		// If it doesn't have the indirection exclamation point, but starts with '${', we'll remove the '${' and the closing '}'
		// TODO(@rail5): This is a hacky way to do this, and should be replaced with a more robust solution
		if (reference_type != bpp::reference_type::ref_method) {
			if (address.substr(0, 3) == "${!") {
				address = "${" + address.substr(3);
			} else if (address.substr(0, 2) == "${") {
				address = address.substr(2, address.size() - 3);
			}
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
