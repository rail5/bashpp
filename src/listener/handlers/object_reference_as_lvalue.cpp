/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_syntax_errors
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

	BashppParser::Ref_lvalueContext* parent = dynamic_cast<BashppParser::Ref_lvalueContext*>(ctx->parent);
	if (parent == nullptr) {
		throw internal_error("Object reference as lvalue context has no parent", ctx);
	}
	bool hasPoundKey = parent->POUNDKEY() != nullptr;

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER_LVALUE(), "Object reference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_class> current_class = current_code_entity->get_containing_class().lock();

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	object_reference_entity->set_containing_class(current_code_entity->get_containing_class());
	object_reference_entity->inherit(current_code_entity);
	entity_stack.push(object_reference_entity);

	std::deque<antlr4::tree::TerminalNode*> ids;

	ids.push_back(ctx->IDENTIFIER_LVALUE());

	for (auto& id : ctx->IDENTIFIER()) {
		ids.push_back(id);
	}

	bpp::entity_reference ref = bpp::resolve_reference(
		current_code_entity,
		ids,
		program
	);

	if (ref.error.has_value()) {
		entity_stack.pop();
		throw_syntax_error(ref.error->token, ref.error->message);
	}

	object_reference_entity->add_code_to_previous_line(ref.reference_code.pre_code);
	object_reference_entity->add_code_to_next_line(ref.reference_code.post_code);

	bpp::reference_type reference_type;

	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(ref.entity);
	std::shared_ptr<bpp::bpp_datamember> datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(ref.entity);
	std::shared_ptr<bpp::bpp_object> object = std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity);

	if (method != nullptr) {
		reference_type = bpp::reference_type::ref_method;
	} else if (datamember != nullptr) {
		reference_type = datamember->get_class() == program->get_primitive_class()
			? bpp::reference_type::ref_primitive
			: bpp::reference_type::ref_object;
	} else if (object != nullptr) {
		reference_type = bpp::reference_type::ref_object;
	} else {
		throw internal_error("Referenced entity is not an object, datamember or method", ctx);
	}

	std::string encase_open, encase_close, indirection;

	object_reference_entity->set_reference_type(reference_type);

	// Are we in an object_assignment context?
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(current_code_entity);

	// Now that we've finished iterating through the identifiers
	encase_open = ref.created_first_temporary_variable ? "${" : "";
	encase_close = ref.created_first_temporary_variable ? "}" : "";
	indirection = ref.created_second_temporary_variable ? "!" : "";

	if (object_assignment != nullptr) {
		// Special encasing rules for object assignments
		encase_open = ref.created_second_temporary_variable ? "${" : "";
		encase_close = ref.created_second_temporary_variable ? "}" : "";
		indirection = "";
	}

	if (reference_type == bpp::reference_type::ref_method) {
		if (object_assignment != nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER_LVALUE(), "Cannot assign to a method");
		}
		// Call the method directly -- not in a supershell
		code_segment method_call = generate_method_call_code(encase_open + indirection + ref.reference_code.code + encase_close, method->get_name(), ref.class_containing_the_method, false, program);

		object_reference_entity->add_code_to_previous_line(method_call.pre_code);
		object_reference_entity->add_code_to_next_line(method_call.post_code);
		object_reference_entity->add_code(method_call.code);
		return;
	}

	// Are we accessing an index of an array?
	if (ctx->array_index() != nullptr) {
		std::string counting = hasPoundKey ? "#" : "";

		bool have_to_dereference_a_pointer = ctx->IDENTIFIER().size() > 1;

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

	bool datamember_is_pointer = (datamember != nullptr) && (datamember->is_pointer());
	std::shared_ptr<bpp::bpp_object> last_reference_object = std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity);
	std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(current_code_entity);

	if (reference_type == bpp::reference_type::ref_primitive || datamember_is_pointer) {
		if (hasPoundKey) {
			indirection = "";
		}
		object_reference_entity->add_code(encase_open + indirection + ref.reference_code.code + encase_close);

		if (delete_entity != nullptr && datamember_is_pointer) {

			delete_entity->set_object_to_delete(last_reference_object);
			delete_entity->set_force_pointer(true);
		}
		return;
	}

	if (ref.entity->get_class() == nullptr) {
		throw internal_error("Last reference entity has no class", ctx);
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
				object_reference_entity->add_code(encase_open + indirection + ref.reference_code.code + encase_close);
			} else {
				// Call .toPrimitive
				code_segment method_call_code = generate_method_call_code(encase_open + indirection + ref.reference_code.code + encase_close, "toPrimitive", last_reference_object->get_class(), false, program);

				object_reference_entity->add_code_to_previous_line(method_call_code.pre_code);
				object_reference_entity->add_code_to_next_line(method_call_code.post_code);
				object_reference_entity->add_code(method_call_code.code);
			}
			return;
		} else {
			object_reference_entity->add_code(encase_open + indirection + ref.reference_code.code + encase_close);
			return;
		}
	}

	if (object_assignment != nullptr) {
		if (last_reference_object != nullptr && (!last_reference_object->is_pointer() || pointer_dereference != nullptr)) {
			object_assignment->set_lvalue_nonprimitive(true);
			object_assignment->set_lvalue_object(last_reference_object);
		}
		object_reference_entity->add_code(encase_open + indirection + ref.reference_code.code + encase_close);
		return;
	}

	if (last_reference_object != nullptr) {
		// Call .toPrimitive
		code_segment method_call_code = generate_method_call_code(encase_open + indirection + ref.reference_code.code + encase_close, "toPrimitive", last_reference_object->get_class(), false, program);

		object_reference_entity->add_code_to_previous_line(method_call_code.pre_code);
		object_reference_entity->add_code_to_next_line(method_call_code.post_code);
		object_reference_entity->add_code(method_call_code.code);
	}
}

void BashppListener::exitObject_reference_as_lvalue(BashppParser::Object_reference_as_lvalueContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());

	if (object_reference_entity == nullptr) {
		throw internal_error("Object reference entity not found on the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found on the entity stack", ctx);
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
