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

	std::shared_ptr<bpp::bpp_object_reference> self_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	self_reference_entity->set_containing_class(current_class);
	self_reference_entity->inherit(current_code_entity);
	entity_stack.push(self_reference_entity);

	if (current_class == nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->KEYWORD_THIS(), "Self reference outside of class");
	}

	self_reference_entity->add_code_to_previous_line("local __this=${__objectAddress}\n");
	self_reference_entity->add_code_to_next_line("unset __this\n");
}

void BashppListener::exitSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	BashppParser::Ref_rvalueContext* parent = dynamic_cast<BashppParser::Ref_rvalueContext*>(ctx->parent);
	bool hasPoundKey = parent->POUNDKEY() != nullptr;

	std::shared_ptr<bpp::bpp_object_reference> self_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());
	if (self_reference_entity == nullptr) {
		throw internal_error("Self reference context was not found in the entity stack", ctx);
	}
	entity_stack.pop();

	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	// Check if we're in an object_address context
	// This will be important later -- we'll have to return differently if we are
	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::dynamic_pointer_cast<bpp::bpp_object_address>(entity_stack.top());

	// Check if we're in a value_assignment context
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());

	std::string self_reference_code = "__this";

	bpp::reference_type last_reference_type = bpp::reference_type::ref_object;
	std::shared_ptr<bpp::bpp_entity> last_reference_entity = current_class;

	std::shared_ptr<bpp::bpp_datamember> datamember = nullptr;
	bool datamember_is_pointer = false;
	std::shared_ptr<bpp::bpp_method> method = nullptr;
	std::shared_ptr<bpp::bpp_class> class_containing_the_method = current_class;

	bool created_first_temporary_variable = false;
	std::string indirection = "";

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

		// Verify that the given identifier is a member of the last reference entity
		datamember = last_reference_entity->get_class()->get_datamember(identifier_text, current_class);
		method = last_reference_entity->get_class()->get_method(identifier_text, current_class);

		if (datamember == bpp::inaccessible_datamember || method == bpp::inaccessible_method) {
			throw_syntax_error_from_exitRule(identifier, identifier_text + " is inaccessible in this context");
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

			indirection = created_first_temporary_variable ? "!" : "";
			std::string temporary_variable_lvalue = self_reference_code + "__" + identifier_text;
			std::string temporary_variable_rvalue = "${" + indirection + self_reference_code + "}__" + identifier_text;

			self_reference_entity->add_code_to_previous_line("local " + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			self_reference_code = temporary_variable_lvalue;
			created_first_temporary_variable = true;
		} else {
			throw_syntax_error_from_exitRule(identifier, last_reference_entity->get_name() + " has no member named " + identifier_text);
		}
	}

	self_reference_entity->set_reference_type(last_reference_type);

	bool ready_to_exit = false;

	if (last_reference_type == bpp::reference_type::ref_method) {
		// Call the method in a supershell, and substitute the result in place of the self-reference

		std::string method_call = "bpp__" + class_containing_the_method->get_name() + "__" + method->get_name() + " ";
		// Append the containing object's address to the method call
		indirection = ctx->IDENTIFIER().size() > 1 ? "!" : "";
		method_call += "${" + indirection + self_reference_code + "}";
		// Tell the method that we *are* passing a pointer
		method_call += " 1";

		code_segment method_code = generate_supershell_code(method_call);
		self_reference_entity->add_code_to_previous_line(method_code.pre_code);
		self_reference_entity->add_code_to_next_line(method_code.post_code);
		self_reference_entity->add_code(method_code.code);
		ready_to_exit = true;
	}

	if (!ready_to_exit) {
		// Are we accessing an index of an array?
		if (ctx->array_index() != nullptr) {
			// We're accessing an array index
			// Either:
			//   1. ctx->AT(1) is set
			//   2. ctx->BASH_VAR() is set
			//   3. ctx->NUMBER() is set

			std::string temporary_variable_lvalue = self_reference_code + "____arrayIndex";
			std::string temporary_variable_rvalue = "${" + self_reference_code + "}[";
			temporary_variable_rvalue += self_reference_entity->get_array_index();
			temporary_variable_rvalue += "]";

			self_reference_entity->add_code_to_previous_line("local " + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			self_reference_code = temporary_variable_lvalue;

			if (hasPoundKey) {
				// Getting the length
				temporary_variable_lvalue = self_reference_code + "____arrayLengthString";
				temporary_variable_rvalue = "\\${#${" + self_reference_code + "}}";
				self_reference_entity->add_code_to_previous_line("local " + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
				self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

				temporary_variable_lvalue = self_reference_code + "____arrayLength";
				temporary_variable_rvalue = "${" + self_reference_code + "____arrayLengthString}";
				self_reference_entity->add_code_to_previous_line("eval local \"" + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\"\n");
				self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

				self_reference_code = temporary_variable_lvalue;
			}
		}

		if (last_reference_entity->get_class() == primitive || last_reference_entity == current_class || datamember_is_pointer) {
			// If the last reference entity is a primitive, simply output the primitive
			// If last_reference_entity == current_class, then the self-reference is a pointer to the object itself (simply @this)
			// Which is also a primitive, so we follow the same procedure
			indirection = (created_first_temporary_variable && !hasPoundKey) ? "!" : "";

			// Are we dereferencing a pointer?
			std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference = std::dynamic_pointer_cast<bpp::bpp_pointer_dereference>(current_code_entity);

			if (pointer_dereference != nullptr && last_reference_entity->get_class() != primitive) {
				// Are we in a value assignment?
				if (value_assignment_entity != nullptr) {
					// TODO(@rail5): Non-primitive copies
					return;
				}
				// Call .toPrimitive in a supershell and substitute the result
				code_segment method_call_code = generate_method_call_code(self_reference_code, "toPrimitive", last_reference_entity->get_class());

				code_segment method_code = generate_supershell_code(method_call_code.pre_code + "\n" + method_call_code.code + "\n" + method_call_code.post_code);
				self_reference_entity->add_code_to_previous_line(method_code.pre_code);
				self_reference_entity->add_code_to_next_line(method_code.post_code);
				self_reference_entity->add_code(method_code.code);

				current_code_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
				current_code_entity->add_code_to_next_line(self_reference_entity->get_post_code());
				current_code_entity->add_code(self_reference_entity->get_code());

				// Show a warning if we're doing a dynamic_cast
				std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(current_code_entity);
				if (dynamic_cast_entity != nullptr) {
					show_warning(ctx->KEYWORD_THIS(), "Dynamic casting the result of .toPrimitive may not be what you want\nDid you mean to take the address of the object?");
				}
				return;
			}

			self_reference_entity->add_code("${" + indirection + self_reference_code + "}");
			ready_to_exit = true;

			// Are we in a delete statement?
			std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
			if (delete_entity != nullptr) {
				if (last_reference_entity->get_class() == primitive) {
					throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot call @delete on a primitive");
				}
				if (last_reference_entity == current_class) {
					throw_syntax_error_from_exitRule(ctx->KEYWORD_THIS(), "Cannot call @delete on '@this'");
				}

				delete_entity->set_object_to_delete(std::dynamic_pointer_cast<bpp::bpp_object>(last_reference_entity));
				delete_entity->set_force_pointer(true);
				delete_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
				delete_entity->add_code_to_next_line(self_reference_entity->get_post_code());
				delete_entity->add_code("${!" + self_reference_code + "}");
				return;
			}
		}
	}

	if (!ready_to_exit) {
		if (last_reference_entity->get_class() == nullptr) {
			throw internal_error("Last reference entity has no class", ctx);
		}

		// Are we otherwise in an object_address context?
		if (object_address_entity != nullptr) {
			self_reference_entity->add_code("${!" + self_reference_code + "}");
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		// If we're here, the last reference entity is a non-primitive object
		if (value_assignment_entity != nullptr && value_assignment_entity->lvalue_is_nonprimitive()) {
			// If we're in a value_assignment context, set the nonprimitive object and set the nonprimitive flag
			value_assignment_entity->set_nonprimitive_object(last_reference_entity);
			value_assignment_entity->set_nonprimitive_assignment(true);
			self_reference_entity->add_code("${!" + self_reference_code + "}");
			ready_to_exit = true;
		}

		// Are we in a delete statement?
		std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
		if (delete_entity != nullptr) {
			if (self_reference_entity->get_reference_type() == bpp::reference_type::ref_method) {
				throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot call @delete on a method");
			}

			if (self_reference_entity->get_reference_type() == bpp::reference_type::ref_primitive) {
				throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot call @delete on a primitive");
			}

			delete_entity->set_object_to_delete(std::dynamic_pointer_cast<bpp::bpp_object>(last_reference_entity));
			delete_entity->set_force_pointer(true);
			delete_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
			delete_entity->add_code_to_next_line(self_reference_entity->get_post_code());
			delete_entity->add_code("${!" + self_reference_code + "}");
			return;
		}
	}

	if (!ready_to_exit) {
		// We need to call the .toPrimitive method on the object
		indirection = (created_first_temporary_variable && !hasPoundKey) ? "!" : "";
		code_segment method_call_code = generate_method_call_code("${" + indirection + self_reference_code + "}", "toPrimitive", last_reference_entity->get_class());

		code_segment method_code = generate_supershell_code(method_call_code.pre_code + "\n" + method_call_code.code + "\n" + method_call_code.post_code);
		self_reference_entity->add_code_to_previous_line(method_code.pre_code);
		self_reference_entity->add_code_to_next_line(method_code.post_code);
		self_reference_entity->add_code(method_code.code);

		// Show a warning if we're doing a dynamic_cast
		std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(current_code_entity);
		if (dynamic_cast_entity != nullptr) {
			show_warning(ctx->KEYWORD_THIS(), "Dynamic casting the result of .toPrimitive may not be what you want\nDid you mean to take the address of the object?");
		}
	}

	// Ready to exit

	// Are we in an object_address context?
	if (object_address_entity != nullptr) {
		if (self_reference_entity->get_reference_type() == bpp::reference_type::ref_method) {
			throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot get the address of a method");
		}
		std::string address = self_reference_entity->get_code();
		// Some hacky string manipulation to work backwards
		// If it starts with '${!', we'll change that to '${'
		// If it doesn't have the indirection exclamation point, but starts with '${', we'll remove the '${' and the closing '}'
		// TODO(@rail5): This is a hacky way to do this, and should be replaced with a more robust solution
		if (address.substr(0, 3) == "${!") {
			address = "${" + address.substr(3);
		} else if (address.substr(0, 2) == "${") {
			address = address.substr(2, address.size() - 3);
		}

		object_address_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
		object_address_entity->add_code_to_next_line(self_reference_entity->get_post_code());
		object_address_entity->add_code(address);
		return;
	}

	// If we're not in any broader context, simply add the object reference to the current code entity
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(self_reference_entity->get_post_code());
		current_code_entity->add_code(self_reference_entity->get_code());
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
