/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_SELF_REFERENCE_AS_LVALUE_CPP_
#define SRC_LISTENER_HANDLERS_SELF_REFERENCE_AS_LVALUE_CPP_

#include "../BashppListener.h"

void BashppListener::enterSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Self references take the form
	 * 	@this.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either an object or a method
	 * If it's a primitive object, replace the reference with the address of the primitive object
	 * If it's a non-primitive object, this is a method call to .toPrimitive
	 * If it's a method, replace the reference with a call to the method
	 * There is no need to call the method in a supershell for lvalues. We can (and must) just call it directly
	 */

	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();
	if (current_class == nullptr) {
		throw_syntax_error(ctx->KEYWORD_THIS_LVALUE(), "Self reference outside of class");
	}

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->AT(), "Object reference outside of code entity");
	}

	// Are we in an object_assignment context?
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());

	// Are we in an object_address context?
	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::dynamic_pointer_cast<bpp::bpp_object_address>(entity_stack.top());

	std::shared_ptr<bpp::bpp_object_reference> self_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	self_reference_entity->set_containing_class(current_class);
	self_reference_entity->inherit(current_code_entity);
	entity_stack.push(self_reference_entity);

	self_reference_entity->add_code_to_previous_line("local __this=${__objectAddress}\n");
	self_reference_entity->add_code_to_next_line("unset __this\n");

	std::string self_reference_code = "__this";

	bpp::reference_type last_reference_type = bpp::reference_type::ref_object;
	std::shared_ptr<bpp::bpp_entity> last_reference_entity = current_class;

	std::shared_ptr<bpp::bpp_datamember> datamember = nullptr;
	bool datamember_is_pointer = false;
	std::shared_ptr<bpp::bpp_method> method = nullptr;
	std::shared_ptr<bpp::bpp_class> class_containing_the_method = current_class;

	bool created_first_temporary_variable = false;

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
				entity_stack.pop();
				throw internal_error("Unknown reference type");
		}

		if (throw_error) {
			entity_stack.pop();
			throw_syntax_error(identifier, error_string);
		}

		std::string identifier_text = identifier->getText();

		// Verify that the given identifier is a member of the last reference entity
		datamember = last_reference_entity->get_class()->get_datamember(identifier_text, current_class);
		method = last_reference_entity->get_class()->get_method(identifier_text, current_class);

		if (datamember == bpp::inaccessible_datamember || method == bpp::inaccessible_method) {
			entity_stack.pop();
			throw_syntax_error(identifier, identifier_text + " is inaccessible in this context");
		}

		if (method != nullptr) {
			// Get the class containing the method
			class_containing_the_method = last_reference_entity->get_class();
			// Update the last reference entity and type
			last_reference_type = bpp::reference_type::ref_method;
			last_reference_entity = method;
		} else if (datamember != nullptr) {
			bool is_primitive = datamember->get_class() == primitive;
			datamember_is_pointer = datamember->is_pointer();
			last_reference_type = is_primitive ? bpp::reference_type::ref_primitive : bpp::reference_type::ref_object;
			last_reference_entity = datamember;

			std::string indirection = created_first_temporary_variable ? "!" : "";
			std::string temporary_variable_lvalue = self_reference_code + "__" + identifier_text;
			std::string temporary_variable_rvalue = "${" + indirection + self_reference_code + "}__" + identifier_text;

			self_reference_entity->add_code_to_previous_line("local " + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			self_reference_code = temporary_variable_lvalue;
			created_first_temporary_variable = true;
		} else {
			entity_stack.pop();
			throw_syntax_error(identifier, last_reference_entity->get_name() + " has no member named " + identifier_text);
		}
	}

	self_reference_entity->set_reference_type(last_reference_type);

	if (last_reference_type == bpp::reference_type::ref_method) {
		// Check if we're in an object assignment context
		// If so, throw a syntax error
		if (object_assignment != nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER().back(), "Cannot assign to a method");
		}

		// Check if we're in an object address context
		// If so, throw a syntax error
		if (object_address_entity != nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER().back(), "Cannot take the address of a method");
		}

		std::string method_call = "bpp__" + class_containing_the_method->get_name() + "__" + method->get_name() + " ";
		// Append the containing object's address to the method call
		std::string indirection = ctx->IDENTIFIER().size() > 1 ? "!" : "";
		method_call += "${" + indirection + self_reference_code + "}";
		// Tell the method that we *are* passing a pointer
		method_call += " 1";

		// Don't run the method in a supershell, just call it directly
		self_reference_entity->add_code(method_call);
		return;
	}

	if (last_reference_type == bpp::reference_type::ref_primitive || last_reference_entity == current_class || datamember_is_pointer) {
		// Always add indirection if it's not an object assignment,
		// Never add indirection if it *is* an object assignment
		std::string indirection = object_assignment == nullptr ? "!" : "";
		self_reference_entity->add_code("${" + indirection + self_reference_code + "}");

		// If we're in an object assignment, and the reference is just '@this', throw a syntax error
		if (object_assignment != nullptr && last_reference_entity == current_class) {
			entity_stack.pop();
			throw_syntax_error(ctx->KEYWORD_THIS_LVALUE(), "Cannot assign to @this");
		}
		return;
	}
	
	if (last_reference_entity->get_class() == nullptr) {
		throw internal_error("Last reference entity has no class");
	}

	// Are we otherwise in an object_address context?
	if (object_address_entity != nullptr) {
		self_reference_entity->add_code("${!" + self_reference_code + "}");
		return;
	}

	// If we're here, the last reference entity is a non-primitive object

	// Are we in an object assignment context?
	if (object_assignment != nullptr) {
		object_assignment->set_lvalue_nonprimitive(true);
		object_assignment->set_lvalue_object(last_reference_entity);
		self_reference_entity->add_code("${!" + self_reference_code + "}");
		return;
	}

	// We need to call the .toPrimitive method on the object
	std::string method_call = "bpp__" + last_reference_entity->get_class()->get_name() + "__toPrimitive ";
	// Append the containing object's address to the method call
	method_call += "${" + self_reference_code + "}";
	// Tell the method that we *are* passing a pointer
	method_call += " 1";

	// Don't run the method in a supershell, just call it directly
	self_reference_entity->add_code(method_call);
}

void BashppListener::exitSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_object_reference> self_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());
	entity_stack.pop();
	if (self_reference_entity == nullptr) {
		throw internal_error("Self reference context was not found in the entity stack");
	}

	// If we're in an object assignment, set the lvalue to the object reference code
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
	if (object_assignment != nullptr) {
		object_assignment->add_code_to_previous_line(self_reference_entity->get_pre_code());
		object_assignment->add_code_to_next_line(self_reference_entity->get_post_code());
		object_assignment->set_lvalue(self_reference_entity->get_code());
		return;
	}

	// If we're not in any broader context, simply add the object reference to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(self_reference_entity->get_post_code());
		current_code_entity->add_code(self_reference_entity->get_code());
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_SELF_REFERENCE_AS_LVALUE_CPP_
