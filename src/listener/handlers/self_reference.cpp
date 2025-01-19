/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
#define SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Self references take the form
	 * 	@this.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either an object or a method
	 * If it's a primitive object, treat this as an rvalue and get the value of the primitive object
	 * If it's a non-primitive object, this is a method call to .toPrimitive
	 * If it's a method, call the method in a supershell and substitute the result
	 */
	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	std::shared_ptr<bpp::bpp_string> self_reference_entity = std::make_shared<bpp::bpp_string>();
	self_reference_entity->set_containing_class(current_class);
	self_reference_entity->inherit(current_code_entity);
	entity_stack.push(self_reference_entity);

	if (current_class == nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->KEYWORD_THIS(), "Self reference outside of class");
	}

	self_reference_entity->add_code_to_previous_line("__this=${__objectAddress}\n");
	self_reference_entity->add_code_to_next_line("unset __this\n");

	std::string self_reference_code = "__this";

	enum reference_type {
		ref_primitive,
		ref_method,
		ref_object
	};

	reference_type last_reference_type = reference_type::ref_object;
	std::shared_ptr<bpp::bpp_entity> last_reference_entity = current_class;

	std::shared_ptr<bpp::bpp_datamember> datamember = nullptr;
	std::shared_ptr<bpp::bpp_method> method = nullptr;
	std::shared_ptr<bpp::bpp_class> class_containing_the_method = current_class;

	bool created_first_temporary_variable = false;
	std::string indirection = "";

	for (auto& identifier : ctx->IDENTIFIER()) {
		bool throw_error = false;
		std::string error_string = "";
		switch (last_reference_type) {
			case reference_type::ref_object:
				break;
			case reference_type::ref_primitive:
				throw_error = true;
				error_string = "Unexpected identifier after primitive object reference";
				break;
			case reference_type::ref_method:
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
			last_reference_type = reference_type::ref_method;
			last_reference_entity = method;
		} else if (datamember != nullptr) {
			bool is_primitive = datamember->get_class() == primitive;
			last_reference_type = is_primitive ? reference_type::ref_primitive : reference_type::ref_object;
			last_reference_entity = datamember;

			indirection = created_first_temporary_variable ? "!" : "";
			std::string temporary_variable_lvalue = self_reference_code + "__" + identifier_text;
			std::string temporary_variable_rvalue = "${" + indirection + self_reference_code + "}__" + identifier_text;

			self_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			self_reference_code = temporary_variable_lvalue;
			created_first_temporary_variable = true;
		} else {
			entity_stack.pop();
			throw_syntax_error(identifier, last_reference_entity->get_name() + " has no member named " + identifier_text);
		}
	}

	if (last_reference_type == reference_type::ref_method) {
		// Call the method in a supershell, and substitute the result in place of the self-reference

		std::string method_call = "bpp__" + class_containing_the_method->get_name() + "__" + method->get_name() + " ";
		// Append the containing object's address to the method call
		method_call += "${" + self_reference_code + "}";
		// Tell the method that we *are* passing a pointer
		method_call += " 1";

		supershell_code method_code = generate_supershell_code(method_call);
		self_reference_entity->add_code_to_previous_line(method_code.pre_code);
		self_reference_entity->add_code_to_next_line(method_code.post_code);
		self_reference_entity->add_code(method_code.code);
		return;
	}

	if (last_reference_entity->get_class() == primitive || last_reference_entity == current_class) {
		// If the last reference entity is a primitive, simply output the primitive
		// If last_reference_entity == current_class, then the self-reference is a pointer to the object itself (simply @this)
		// Which is also a primitive, so we follow the same procedure
		indirection = created_first_temporary_variable ? "!" : "";
		self_reference_entity->add_code("${" + indirection + self_reference_code + "}");
		return;
	}

	if (last_reference_entity->get_class() == nullptr) {
		throw internal_error("Last reference entity has no class");
	}

	// If we're here, the last reference entity is a non-primitive object
	// We need to call the .toPrimitive method on the object
	std::string method_call = "bpp__" + last_reference_entity->get_class()->get_name() + "__toPrimitive ";
	// Append the containing object's address to the method call
	method_call += "${" + self_reference_code + "}";
	// Tell the method that we *are* passing a pointer
	method_call += " 1";

	supershell_code method_code = generate_supershell_code(method_call);
	self_reference_entity->add_code_to_previous_line(method_code.pre_code);
	self_reference_entity->add_code_to_next_line(method_code.post_code);
	self_reference_entity->add_code(method_code.code);

}

void BashppListener::exitSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> self_reference_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	entity_stack.pop();
	if (self_reference_entity == nullptr) {
		throw internal_error("Self reference context was not found in the entity stack");
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

#endif // SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
