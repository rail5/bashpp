/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterObjectReference(std::shared_ptr<AST::ObjectReference> node) {
	skip_syntax_errors
	/**
	 * Object references take the form
	 * 	[*|&]@IDENTIFIER.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either an object or a method
	 * If it's a primitive object, treat this as an rvalue and get the value of the primitive object
	 * If it's a non-primitive object, this is a method call to .toPrimitive
	 * If it's a method, call the method in a supershell and substitute the result
	 *
	 * This rule handles:
	 * - Normal object references (@obj.member)
	 * - Self-references (@this.member or @super.member)
	 * - Pointer dereferences (*@ptr)
	 * - Object addresses (&@obj)
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		syntax_error(node, "Object reference outside of code entity");
	}

	std::shared_ptr<bpp::bpp_class> current_class = current_code_entity->get_containing_class().lock();

	std::shared_ptr<bpp::bpp_object_reference> object_reference_entity = std::make_shared<bpp::bpp_object_reference>();
	object_reference_entity->set_containing_class(current_code_entity->get_containing_class());
	object_reference_entity->inherit(current_code_entity);
	entity_stack.push(object_reference_entity);
}

void BashppListener::exitObjectReference(std::shared_ptr<AST::ObjectReference> node) {
	skip_syntax_errors
	auto object_reference_entity = std::dynamic_pointer_cast<bpp::bpp_object_reference>(entity_stack.top());
	if (object_reference_entity == nullptr) {
		throw internal_error("Object reference context was not found in the entity stack");
	}
	entity_stack.pop();

	auto current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack");
	}

	auto current_class = current_code_entity->get_containing_class().lock();

	// @super forces static method resolution
	bool force_static_resolution = node->IDENTIFIER().getValue() == "super";

	bool lvalue = node->isLvalue();
	bool self_reference = node->isSelfReference();
	bool pointer_dereference = node->isPointerDereference();
	bool object_address = node->isAddressOf();
	/**
	 * There are 12 possible combinations here:
	 *		LVALUE	SELF_REF	POINTER_DEREF	OBJ_ADDRESS
	 * 1.	0		0			0				0			-> Object reference as rvalue
	 * 2.	0		0			0				1			-> Object address as rvalue
	 * 3.	0		0			1				0			-> Pointer dereference as rvalue
	 * 4.	0		1			0				0			-> Self reference as rvalue
	 * 5.	0		1			0				1			-> Self reference as rvalue (address of)
	 * 6.	0		1			1				0			-> Self reference as rvalue (pointer deref)
	 * 7.	1		0			0				0			-> Object reference as lvalue
	 * 8.	1		0			0				1			-> Object address as lvalue
	 * 9.	1		0			1				0			-> Pointer dereference as lvalue
	 * 10.	1		1			0				0			-> Self reference as lvalue
	 * 11.	1		1			0				1			-> Self reference as lvalue (address of)
	 * 12.	1		1			1				0			-> Self reference as lvalue (pointer deref)
	 *
	 * POINTER_DEREF and OBJ_ADDRESS are mutually exclusive, so combinations where both are 1 are invalid
	 * 
	 */
	
	if (pointer_dereference && object_address) {
		throw internal_error("Detected simultaneous pointer dereference and object address");
	}

	std::deque<AST::Token<std::string>> ids;
	ids.push_back(node->IDENTIFIER());
	for (const auto& id : node->IDENTIFIERS()) {
		ids.push_back(id);
	}

	bpp::entity_reference ref = bpp::resolve_reference(
		source_file,
		current_code_entity,
		&ids,
		should_declare_local(),
		program
	);

	if (ref.error.has_value()) {
		syntax_error(ref.error->token, ref.error->message);
	}

	bpp::reference_type reference_type;

	auto method = std::dynamic_pointer_cast<bpp::bpp_method>(ref.entity);
	auto datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(ref.entity);
	auto object = std::dynamic_pointer_cast<bpp::bpp_object>(ref.entity);
	auto class_ = std::dynamic_pointer_cast<bpp::bpp_class>(ref.entity);

	if (method != nullptr) {
		reference_type = bpp::reference_type::ref_method;
	} else if (datamember != nullptr) {
		reference_type = (datamember->get_class() == program->get_primitive_class())
			? bpp::reference_type::ref_primitive
			: bpp::reference_type::ref_object;
	} else if (object != nullptr) {
		reference_type = bpp::reference_type::ref_object;
	} else if (self_reference && class_ != nullptr) {
		// Self-reference to a class (eg, @this or @super)
		// Treat this as an object reference, create a temporary object representing "this"
		reference_type = bpp::reference_type::ref_object;
		object = std::make_shared<bpp::bpp_object>();
		object->set_class(class_); // Could be either current_class or its parent class if @super
		object->set_address("__this"); // CAREFUL: This relies on the this pointer being stored in __this
		object->set_pointer(true);

		// Extra checks:
		auto object_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
		if (object_assignment_entity != nullptr) {
			syntax_error(node, "Cannot assign to '@this'");
		}
	} else {
		throw internal_error("Referenced entity is not an object, datamember or method");
	}

	object_reference_entity->set_reference_type(reference_type);

	// First, determine: If the final-referenced entity is a pointer to a non-primitive object,
	// And pointer_dereference == true,
	// Then from here on we should treat that pointer as though it were the object it refers to directly
	if (reference_type == bpp::reference_type::ref_object
		&& object->is_pointer()
		&& pointer_dereference
	) {
		// Create a temporary copy of the object entity that is not marked as a pointer
		// This is sufficient because the runtime automatically dereferences pointers as needed
		// So the only thing we need to change is how we treat this entity within the compiler, not within the runtime
		auto dereferenced_object = std::make_shared<bpp::bpp_object>(*object);
		dereferenced_object->set_pointer(false);
		// Work with this copy from here on
		ref.entity = dereferenced_object;
		object = dereferenced_object;
	}

	// Next, determine: Have we referenced a non-primitive object in a place where a primitive is expected?
	// If so, we need to replace the referenced entity with the object's .toPrimitive method entity
	if (reference_type == bpp::reference_type::ref_object
		&& !object->is_pointer()
		&& !object_address
		&& !context_expectations_stack.canTakeObject()
	) {
		// Check for a couple cases in which we should emit warnings
		// (i.e., where the user might have INTENDED to take the address of the object, or might not realize that .toPrimitive is being called)
		if (!dynamic_cast_stack.empty()) {
			auto cast_target_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_target>(entity_stack.top());
			if (cast_target_entity != nullptr) {
				// This is the target of a dynamic cast
				// I.e., the "cast-to" portion of a @dynamic_cast<cast-to> statement
				show_warning(node,
					std::string("This will attempt to interpret the output of @" + node->IDENTIFIER().getValue() + ".toPrimitive as a class name at runtime."));
			} else {
				// This is that which is being dynamically cast
				show_warning(node,
					std::string("Dynamic casting the output of @" + node->IDENTIFIER().getValue() + ".toPrimitive may not be what you want.\nDid you mean to take the address of the object?"));
			}
		}

		if (!typeof_stack.empty()) {
			// This is the target of a typeof expression
			show_warning(node,
				std::string("Checking the 'type' of the output of @" + node->IDENTIFIER().getValue() + ".toPrimitive may not be what you want.\nDid you mean to take the address of the object?"));
		}

		// Re-call resolve_reference to get the .toPrimitive method
		std::deque<std::string> to_primitive_ids; // By pushing strings instead of tokens, we avoid duplicating diagnostics & reference counters
		for (const auto& id : ids) {
			to_primitive_ids.push_back(id.getValue());
		}
		to_primitive_ids.push_back("toPrimitive");

		ref = bpp::resolve_reference(
			source_file,
			current_code_entity,
			&to_primitive_ids,
			should_declare_local(),
			program
		);

		reference_type = bpp::reference_type::ref_method;
		object = nullptr;
		method = std::dynamic_pointer_cast<bpp::bpp_method>(ref.entity);
		if (method == nullptr) {
			throw internal_error("toPrimitive method not found for class " + ref.class_containing_the_method->get_name());
		}
	}

	// Add the pre- and post- code necessary to resolve the object reference
	object_reference_entity->add_code_to_previous_line(ref.reference_code.pre_code);
	object_reference_entity->add_code_to_next_line(ref.reference_code.post_code);

	// 1. Is it a method?
	if (reference_type == bpp::reference_type::ref_method) {
		if (ref.reference_code.code == "__this") ref.reference_code.code = "${__this}"; // Kind of a hack to ensure that "this" references work correctly
		auto method_call = bpp::generate_method_call_code(
			ref.reference_code.code,
			method->get_name(),
			ref.class_containing_the_method,
			force_static_resolution,
			program
		);

		// Add the pre- and post- code necessary to call the method
		object_reference_entity->add_code_to_previous_line(method_call.pre_code);
		object_reference_entity->add_code_to_next_line(method_call.post_code);

		std::string code_to_add = method_call.code;

		// If this is an rvalue reference, the method call must be run in a supershell
		if (!lvalue && !object_address) {
			auto supershell = bpp::generate_supershell_code(
				method_call.code,
				program
			);
			// Add the pre- and post- code necessary to run a supershell
			object_reference_entity->add_code_to_previous_line(supershell.pre_code);
			object_reference_entity->add_code_to_next_line(supershell.post_code);

			code_to_add = supershell.code;
		}

		// NOTE: If object_address is true here, we are taking the address of the method
		//
		// The "address" of an object's method is:
		//  - The function name
		//  - The object's address (the "this" pointer) as the method's implicit first argument
		//
		// We do not need to handle it specially in this case, apart from the above check
		// `if (!lvalue && !object_address)`
		// Which ensures that if we are taking the address of the method, we do not run it in a supershell,
		// but instead just output the method call code directly.
		//
		// Incidentally, taking a method's address in an lvalue reference is semantically equivalent
		// to simply calling the method (i.e., to **not** taking its address at all)
		// for the same reason that `echo hello` and `var="echo"; $var hello` are equivalent in bash.

		object_reference_entity->add_code(code_to_add);
	}

	// 2. Is it a non-primitive object?
	if (reference_type == bpp::reference_type::ref_object && !object_address) {
		// Are we in a @delete statement?
		auto delete_entity = std::dynamic_pointer_cast<bpp::bpp_delete_statement>(entity_stack.top());
		if (delete_entity != nullptr) {
			delete_entity->set_object_to_delete(object);
			object_reference_entity->add_code(ref.reference_code.code);
		}

		// Is this the lvalue of an object assignment?
		auto object_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
		if (object_assignment_entity != nullptr && !object->is_pointer()) {
			object_assignment_entity->set_lvalue_object(object);
			object_assignment_entity->set_lvalue_nonprimitive(true);
			object_assignment_entity->set_lvalue(ref.reference_code.code);
			object_reference_entity->add_code(ref.reference_code.code);
		}

		// Is this the rvalue of an object assignment?
		auto value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(entity_stack.top());
		if (value_assignment_entity != nullptr && !object->is_pointer()) {
			value_assignment_entity->set_nonprimitive_object(object);
			value_assignment_entity->set_nonprimitive_assignment(true);
			object_reference_entity->add_code(ref.reference_code.code);
		}
		// That's all 3 of the special cases in which non-primitive objects are directly acceptable.
	}

	// 2.5. Is this simply a request for an object's address?
	if (reference_type == bpp::reference_type::ref_object && object_address && !object->is_pointer()) {
		object_reference_entity->add_code(object->get_address());
	}

	// 3. Is it a primitive? (Or a pointer)
	if (reference_type == bpp::reference_type::ref_primitive
		|| (reference_type == bpp::reference_type::ref_object && object->is_pointer())
	) {

		uint8_t indirection_level = 0;
		if (ref.created_first_temporary_variable) indirection_level++;
		if (ref.created_second_temporary_variable) indirection_level++;


		/* Abandon all hope, ye who enter here */
		std::string counting = node->hasHashkey() ? "#" : "";		
		if (object_reference_entity->has_array_index()) {
			// Special procedure needed to handle array indices
			std::string local_decl = should_declare_local() ? "local " : "";
			std::string indirection = ref.created_second_temporary_variable ? "!" : "";
			std::string temporary_variable_lvalue = ref.reference_code.code + "____arrayIndexString";
			std::string temporary_variable_rvalue;

			bool first_object_is_pointer = false;
			{
				std::deque<std::string> first_id = {node->IDENTIFIER().getValue()};
				auto first_object = std::dynamic_pointer_cast<bpp::bpp_object>(bpp::resolve_reference(
					source_file,
					current_code_entity,
					&first_id,
					should_declare_local(),
					program
				).entity);
				first_object_is_pointer = (first_object != nullptr) && first_object->is_pointer();
			}
			first_object_is_pointer = first_object_is_pointer || self_reference;
			
			bool have_to_dereference_a_pointer = first_object_is_pointer || indirection_level >= 2;

			if (have_to_dereference_a_pointer) {
				temporary_variable_rvalue = counting + "${" + ref.reference_code.code + "}" + object_reference_entity->get_array_index();
			} else {
				// Can grab it directly
				temporary_variable_rvalue = "${" + counting + indirection + ref.reference_code.code +  object_reference_entity->get_array_index() + "}";
			}

			object_reference_entity->add_code_to_previous_line(local_decl + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n");
			object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");

			temporary_variable_rvalue = "${" + ref.reference_code.code + "____arrayIndexString}";

			// If we're counting, another small layer of abstraction is needed
			if (node->hasHashkey()) {
				temporary_variable_rvalue = "\\${" + temporary_variable_rvalue + "}";
			}

			if (have_to_dereference_a_pointer) {
				temporary_variable_lvalue = ref.reference_code.code + "____arrayIndex";
				object_reference_entity->add_code_to_previous_line("eval " + local_decl + temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
				object_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			}

			ref.reference_code.code = temporary_variable_lvalue;
			if (node->hasHashkey() && indirection_level >= 2) indirection_level--;
		}
		/* End of the shameful section */

		if (object_address) indirection_level--;

		std::string encased_reference_code = bpp::get_encased_ref(ref.reference_code.code, indirection_level);

		// Is this the lvalue of an object assignment?
		auto object_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(entity_stack.top());
		if (object_assignment_entity != nullptr) {
			indirection_level--; // Lvalue assignments reduce indirection level by 1
			encased_reference_code = bpp::get_encased_ref(ref.reference_code.code, indirection_level);
			object_assignment_entity->set_lvalue(encased_reference_code);
			object_assignment_entity->set_lvalue_nonprimitive(false);
		}

		object_reference_entity->add_code(encased_reference_code);
	}

	current_code_entity->add_code_to_previous_line(object_reference_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(object_reference_entity->get_post_code());
	current_code_entity->add_code(object_reference_entity->get_code());
}
