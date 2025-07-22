/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) {
	skip_syntax_errors
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

	antlr4::tree::TerminalNode* this_keyword = ctx->KEYWORD_THIS_LVALUE() != nullptr ? ctx->KEYWORD_THIS_LVALUE() : ctx->KEYWORD_SUPER_LVALUE();

	bool force_static_reference = (this_keyword == ctx->KEYWORD_SUPER_LVALUE()); // Whether to skip vtable lookups and resolve methods statically
			// This flag is only tripped if we're calling @super.someMethod
			// In that case, we resolve the method statically

	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();
	if (current_class == nullptr) {
		throw_syntax_error(this_keyword, "Self reference outside of class");
	}

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(this_keyword, "Object reference outside of code entity");
	}

	// Are we in an object_assignment context?
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());

	// Are we in an object_address context?
	std::shared_ptr<bpp::bpp_object_address> object_address_entity = std::dynamic_pointer_cast<bpp::bpp_object_address>(entity_stack.top());

	std::shared_ptr<bpp::bpp_object_reference> self_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	self_reference_entity->set_containing_class(current_class);
	self_reference_entity->inherit(current_code_entity);
	entity_stack.push(self_reference_entity);

	std::deque<antlr4::tree::TerminalNode*> ids;

	ids.push_back(this_keyword);

	for (auto& id : ctx->IDENTIFIER()) {
		ids.push_back(id);
	}

	bpp::entity_reference ref = bpp::resolve_reference(
		source_file,
		current_code_entity,
		&ids,
		program
	);

	if (ref.error.has_value()) {
		entity_stack.pop();
		throw_syntax_error(ref.error->token, ref.error->message);
	}

	self_reference_entity->add_code_to_previous_line(ref.reference_code.pre_code);
	self_reference_entity->add_code_to_next_line(ref.reference_code.post_code);

	bpp::reference_type reference_type = bpp::reference_type::ref_object;

	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(ref.entity);
	std::shared_ptr<bpp::bpp_datamember> datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(ref.entity);
	bool datamember_is_pointer = datamember != nullptr && datamember->is_pointer();

	if (method != nullptr) {
		reference_type = bpp::reference_type::ref_method;
	} else if (datamember != nullptr) {
		reference_type = datamember->get_class() == program->get_primitive_class()
			? bpp::reference_type::ref_primitive
			: bpp::reference_type::ref_object;
	}

	self_reference_entity->set_reference_type(reference_type);

	if (reference_type == bpp::reference_type::ref_method) {
		// Check if we're in an object assignment context
		// If so, throw a syntax error
		if (object_assignment != nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER().back(), "Cannot assign to a method");
		}

		std::string indirection = ctx->IDENTIFIER().size() > 1 ? "!" : "";

		code_segment method_call_code = generate_method_call_code("${" + indirection + ref.reference_code.code + "}", method->get_name(), ref.class_containing_the_method, force_static_reference, program);

		// Don't run the method in a supershell, just call it directly
		self_reference_entity->add_code_to_previous_line(method_call_code.pre_code);
		self_reference_entity->add_code_to_next_line(method_call_code.post_code);
		self_reference_entity->add_code(method_call_code.code);
		return;
	}

	if (reference_type == bpp::reference_type::ref_primitive
		|| ref.entity == current_class
		|| ref.entity == current_class->get_parent()
		|| datamember_is_pointer) {
		// Never add indirection if it's an object assignment
		// Don't add indirection if the last reference entity is the current class
		std::string indirection = (object_assignment == nullptr
				&& ref.entity != current_class
				&& ref.entity != current_class->get_parent()) ? "!" : "";
		self_reference_entity->add_code("${" + indirection + ref.reference_code.code + "}");

		// If we're in an object assignment, and the reference is just '@this', throw a syntax error
		if (object_assignment != nullptr && (ref.entity == current_class || ref.entity == current_class->get_parent())) {
			entity_stack.pop();
			throw_syntax_error(this_keyword, "Cannot assign to @this");
		}

		// Are we dereferencing a pointer?
		std::shared_ptr<bpp::bpp_pointer_dereference> pointer_dereference = std::dynamic_pointer_cast<bpp::bpp_pointer_dereference>(current_code_entity);
		if (pointer_dereference != nullptr && reference_type != bpp::reference_type::ref_primitive) {
			// Call .toPrimitive
			code_segment method_call_code = generate_method_call_code("${" + indirection + ref.reference_code.code + "}", "toPrimitive", ref.entity->get_class(), force_static_reference, program);

			pointer_dereference->add_code_to_previous_line(self_reference_entity->get_pre_code());
			pointer_dereference->add_code_to_previous_line(method_call_code.pre_code);
			pointer_dereference->add_code_to_next_line(self_reference_entity->get_post_code());
			pointer_dereference->add_code_to_next_line(method_call_code.post_code);
			pointer_dereference->add_code(method_call_code.code);

			// Clear the self reference entity's code buffers
			self_reference_entity->clear_all_buffers();
		}
		return;
	}

	if (ref.entity->get_class() == nullptr) {
		throw internal_error("Last reference entity has no class", ctx);
	}

	// Are we otherwise in an object_address context?
	if (object_address_entity != nullptr) {
		self_reference_entity->add_code("${!" + ref.reference_code.code + "}");
		return;
	}

	// If we're here, the last reference entity is a non-primitive object

	// Are we in an object assignment context?
	if (object_assignment != nullptr) {
		object_assignment->set_lvalue_nonprimitive(true);
		object_assignment->set_lvalue_object(ref.entity);
		self_reference_entity->add_code("${!" + ref.reference_code.code + "}");
		return;
	}

	// We need to call the .toPrimitive method on the object
	code_segment method_call_code = generate_method_call_code("${!" + ref.reference_code.code + "}", "toPrimitive", ref.entity->get_class(), force_static_reference, program);

	// Don't run the method in a supershell, just call it directly
	self_reference_entity->add_code_to_previous_line(method_call_code.pre_code);
	self_reference_entity->add_code_to_next_line(method_call_code.post_code);
	self_reference_entity->add_code(method_call_code.code);
}

void BashppListener::exitSelf_reference_as_lvalue(BashppParser::Self_reference_as_lvalueContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_object_reference> self_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());
	entity_stack.pop();
	if (self_reference_entity == nullptr) {
		throw internal_error("Self reference context was not found in the entity stack", ctx);
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
