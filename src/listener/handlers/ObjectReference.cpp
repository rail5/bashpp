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
		throw_syntax_error(node, "Object reference outside of code entity");
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
		throw_syntax_error_from_exitRule(ref.error->token, ref.error->message);
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
		reference_type = (datamember->get_class() == program->get_primitive_class())
			? bpp::reference_type::ref_primitive
			: bpp::reference_type::ref_object;
	} else if (object != nullptr) {
		reference_type = bpp::reference_type::ref_object;
	} else {
		throw internal_error("Referenced entity is not an object, datamember or method");
	}

	object_reference_entity->set_reference_type(reference_type);

	std::string encase_open, encase_close, indirection;
	encase_open = ref.created_first_temporary_variable ? "${" : "";
	encase_close = ref.created_first_temporary_variable ? "}" : "";
	indirection = ref.created_second_temporary_variable ? "!" : "";


}
