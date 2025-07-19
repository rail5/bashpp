/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_syntax_errors
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

	antlr4::tree::TerminalNode* this_keyword = ctx->KEYWORD_THIS() != nullptr ? ctx->KEYWORD_THIS() : ctx->KEYWORD_SUPER();

	if (current_class == nullptr) {
		entity_stack.pop();
		throw_syntax_error(this_keyword, "Self reference outside of class");
	}

}

void BashppListener::exitSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_syntax_errors
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

	antlr4::tree::TerminalNode* this_keyword = ctx->KEYWORD_THIS() != nullptr ? ctx->KEYWORD_THIS() : ctx->KEYWORD_SUPER();

	bool force_static_reference = (this_keyword == ctx->KEYWORD_SUPER()); // Whether to skip vtable lookups and resolve methods statically
			// This flag is only tripped if we're calling @super.someMethod
			// In that case, we resolve the method statically

	// Check if we're in an object_address context
	// This will be important later -- we'll have to return differently if we are
	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::dynamic_pointer_cast<bpp::bpp_object_address>(entity_stack.top());

	// Check if we're in a value_assignment context
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());

	std::deque<antlr4::tree::TerminalNode*> ids;
	ids.push_back(this_keyword);
	for (auto& id : ctx->IDENTIFIER()) {
		ids.push_back(id);
	}

	bpp::entity_reference ref = bpp::resolve_reference(
		current_code_entity,
		ids,
		program
	);

	if (ref.error.has_value()) {
		throw_syntax_error_from_exitRule(ref.error->token, ref.error->message);
	}

	self_reference_entity->add_code_to_previous_line(ref.reference_code.pre_code);
	self_reference_entity->add_code_to_next_line(ref.reference_code.post_code);

	bpp::reference_type reference_type = bpp::reference_type::ref_object;

	std::shared_ptr<bpp::bpp_datamember> datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(ref.entity);
	bool datamember_is_pointer = datamember != nullptr && datamember->is_pointer();
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(ref.entity);
	
	if (method != nullptr) {
		reference_type = bpp::reference_type::ref_method;
	} else if (datamember != nullptr) {
		reference_type = datamember->get_class() == program->get_primitive_class()
			? bpp::reference_type::ref_primitive
			: bpp::reference_type::ref_object;
	}

	self_reference_entity->set_reference_type(reference_type);

	std::string indirection = ref.created_second_temporary_variable ? "!" : "";

	bool ready_to_exit = false;

	if (reference_type == bpp::reference_type::ref_method) {
		code_segment method_call_code = generate_method_call_code("${" + indirection + ref.reference_code.code + "}", method->get_name(), ref.class_containing_the_method, force_static_reference, program);

		// Are we taking the address of the method, or calling it?
		if (object_address_entity != nullptr) {
			self_reference_entity->add_code_to_previous_line(method_call_code.pre_code);
			self_reference_entity->add_code_to_next_line(method_call_code.post_code);
			self_reference_entity->add_code(method_call_code.code);
		} else {
			// Call the method in a supershell, and substitute the result in place of the self-reference
			code_segment method_code = generate_supershell_code(method_call_code.full_code(), in_while_condition, current_while_condition, program);
			self_reference_entity->add_code_to_previous_line(method_code.pre_code);
			self_reference_entity->add_code_to_next_line(method_code.post_code);
			self_reference_entity->add_code(method_code.code);
		}
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

			std::string temporary_variable_lvalue = ref.reference_code.code + "____arrayIndex";
			std::string temporary_variable_rvalue = "${" + ref.reference_code.code + "}[";
			temporary_variable_rvalue += self_reference_entity->get_array_index();
			temporary_variable_rvalue += "]";

			self_reference_entity->add_code_to_previous_line("local " + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			ref.reference_code.code = temporary_variable_lvalue;

			if (hasPoundKey) {
				// Getting the length
				temporary_variable_lvalue = ref.reference_code.code + "____arrayLengthString";
				temporary_variable_rvalue = "\\${#${" + ref.reference_code.code + "}}";
				self_reference_entity->add_code_to_previous_line("local " + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
				self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

				temporary_variable_lvalue = ref.reference_code.code + "____arrayLength";
				temporary_variable_rvalue = "${" + ref.reference_code.code + "____arrayLengthString}";
				self_reference_entity->add_code_to_previous_line("eval local \"" + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\"\n");
				self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

				ref.reference_code.code = temporary_variable_lvalue;
			}
		}

		if (ref.entity->get_class() == primitive || ref.entity == current_class || ref.entity == current_class->get_parent() || datamember_is_pointer) {
			// If the last reference entity is a primitive, simply output the primitive
			// If ref.entity == current_class, then the self-reference is a pointer to the object itself (simply @this)
			// Which is also a primitive, so we follow the same procedure
			indirection = (ref.created_second_temporary_variable && !hasPoundKey) ? "!" : "";

			// Are we dereferencing a pointer?
			std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference = std::dynamic_pointer_cast<bpp::bpp_pointer_dereference>(current_code_entity);

			if (pointer_dereference != nullptr && ref.entity->get_class() != primitive) {
				// Are we in a value assignment?
				if (value_assignment_entity != nullptr) {
					// TODO(@rail5): Non-primitive copies
					return;
				}
				// Call .toPrimitive in a supershell and substitute the result
				code_segment method_call_code = generate_method_call_code("${" + indirection + ref.reference_code.code + "}", "toPrimitive", ref.entity->get_class(), force_static_reference, program);

				code_segment method_code = generate_supershell_code(method_call_code.full_code(), in_while_condition, current_while_condition, program);
				self_reference_entity->add_code_to_previous_line(method_code.pre_code);
				self_reference_entity->add_code_to_next_line(method_code.post_code);
				self_reference_entity->add_code(method_code.code);

				current_code_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
				current_code_entity->add_code_to_next_line(self_reference_entity->get_post_code());
				current_code_entity->add_code(self_reference_entity->get_code());

				// Show a warning if we're doing a dynamic_cast
				std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(current_code_entity);
				if (dynamic_cast_entity != nullptr) {
					show_warning(this_keyword, "Dynamic casting the result of .toPrimitive may not be what you want\nDid you mean to take the address of the object?");
				}
				return;
			}

			self_reference_entity->add_code("${" + indirection + ref.reference_code.code + "}");
			ready_to_exit = true;

			// Are we in a delete statement?
			std::shared_ptr<bpp::bpp_delete_statement> delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
			if (delete_entity != nullptr) {
				if (ref.entity->get_class() == primitive) {
					throw_syntax_error_from_exitRule(ctx->IDENTIFIER().back(), "Cannot call @delete on a primitive");
				}
				if (ref.entity == current_class || ref.entity == current_class->get_parent()) {
					throw_syntax_error_from_exitRule(this_keyword, "Cannot call @delete on '@this'");
				}

				delete_entity->set_object_to_delete(std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity));
				delete_entity->set_force_pointer(true);
				delete_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
				delete_entity->add_code_to_next_line(self_reference_entity->get_post_code());
				delete_entity->add_code("${!" + ref.reference_code.code + "}");
				return;
			}
		}
	}

	if (!ready_to_exit) {
		if (ref.entity->get_class() == nullptr) {
			throw internal_error("Last reference entity has no class", ctx);
		}

		// Are we otherwise in an object_address context?
		if (object_address_entity != nullptr) {
			self_reference_entity->add_code("${!" + ref.reference_code.code + "}");
			ready_to_exit = true;
		}
	}

	if (!ready_to_exit) {
		// If we're here, the last reference entity is a non-primitive object
		if (value_assignment_entity != nullptr && value_assignment_entity->lvalue_is_nonprimitive()) {
			// If we're in a value_assignment context, set the nonprimitive object and set the nonprimitive flag
			value_assignment_entity->set_nonprimitive_object(ref.entity);
			value_assignment_entity->set_nonprimitive_assignment(true);
			self_reference_entity->add_code("${!" + ref.reference_code.code + "}");
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

			delete_entity->set_object_to_delete(std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity));
			delete_entity->set_force_pointer(true);
			delete_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
			delete_entity->add_code_to_next_line(self_reference_entity->get_post_code());
			delete_entity->add_code("${!" + ref.reference_code.code + "}");
			return;
		}
	}

	if (!ready_to_exit) {
		// We need to call the .toPrimitive method on the object
		indirection = (ref.created_second_temporary_variable && !hasPoundKey) ? "!" : "";
		code_segment method_call_code = generate_method_call_code("${" + indirection + ref.reference_code.code + "}", "toPrimitive", ref.entity->get_class(), false, program);

		code_segment method_code = generate_supershell_code(method_call_code.full_code(), in_while_condition, current_while_condition, program);
		self_reference_entity->add_code_to_previous_line(method_code.pre_code);
		self_reference_entity->add_code_to_next_line(method_code.post_code);
		self_reference_entity->add_code(method_code.code);

		// Show a warning if we're doing a dynamic_cast
		std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(current_code_entity);
		if (dynamic_cast_entity != nullptr) {
			show_warning(this_keyword, "Dynamic casting the result of .toPrimitive may not be what you want\nDid you mean to take the address of the object?");
		}
	}

	// Ready to exit

	// Are we in an object_address context?
	if (object_address_entity != nullptr) {
		std::string address = self_reference_entity->get_code();
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
