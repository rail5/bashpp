/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#include "bpp_codegen.h"
#include "bpp.h"

namespace bpp {

/**
 * @brief Generates a supershell code segment for executing a bash command.
 *
 * This function constructs a code segment to run a specified command in a supershell.
 * It creates a unique function name and output variable using a global counter. The generated code includes:
 * - A bash function definition wrapping the given command.
 * - A command to invoke the function and store its output, either appended to a while condition or added to the precode.
 * - Cleanup commands that unset the dynamically created function and output variable.
 *
 * @param code_to_run The bash command to be executed within the supershell.
 *
 * @return A code_segment structure containing the complete supershell execution code:
 *         - pre_code: The setup code including the function definition and invocation.
 *         - post_code: The code for cleaning up the defined environment.
 *         - code: An expression referencing the supershell output variable.
 */
code_segment generate_supershell_code(
	const std::string& code_to_run,
	bool in_while_condition,
	std::shared_ptr<bash_while_condition> current_while_condition,
	std::shared_ptr<bpp::bpp_program> program
) {
	code_segment result;

	uint64_t supershell_counter = program->get_supershell_counter();

	std::string supershell_function_name = "____supershellRunFunc" + std::to_string(supershell_counter);
	std::string supershell_output_variable = "____supershellOutput" + std::to_string(supershell_counter);

	result.pre_code += "function " + supershell_function_name + "() {\n";
	result.pre_code += "	" + code_to_run + "\n";
	result.pre_code += "}\n";

	if (in_while_condition) {
		current_while_condition->add_supershell_function_call("bpp____supershell " + supershell_output_variable + " " + supershell_function_name);
		current_while_condition->increment_supershell_count();
	} else {
		result.pre_code += "bpp____supershell " + supershell_output_variable + " " + supershell_function_name + "\n";
	}
	result.post_code += "unset -f " + supershell_function_name + "\n";
	result.post_code += "unset " + supershell_output_variable + "\n";

	result.code = "${" + supershell_output_variable + "}";

	program->increment_supershell_counter();

	return result;
}

/**
 * @brief Generates a code segment for deleting an object.
 *
 * This function constructs a code segment to delete an object. The generated code includes:
 * - A call to the object's destructor if it has one.
 * - A call to the object's delete function.
 *
 * @param object The object to be deleted.
 * @param object_ref The string representing the object's reference in the compiled code.
 *
 * @return A code_segment structure containing the complete deletion code:
 *         - pre_code: The setup code including the destructor call.
 *         - post_code: Empty
 *         - code: Empty
 */
code_segment generate_delete_code(
	std::shared_ptr<bpp::bpp_object> object,
	const std::string& object_ref,
	std::shared_ptr<bpp::bpp_program> program
) {
	// The object_ref is how the compiled code should refer to the object
	// Ie, if the object is a pointer, this should be the address of the object
	code_segment result;

	code_segment destructor_code = generate_method_call_code(object_ref, "__destructor", object->get_class(), false, program);
	result.pre_code += destructor_code.pre_code;
	result.pre_code += destructor_code.code + "\n";
	result.pre_code += destructor_code.post_code;

	code_segment delete_code = generate_method_call_code(object_ref, "__delete", object->get_class(), false, program);
	result.pre_code += delete_code.pre_code;
	result.pre_code += delete_code.code + "\n";
	result.pre_code += delete_code.post_code;

	return result;
}

/**
 * @brief Generates a code segment for calling a method.
 *
 * This function constructs a code segment to call a method on an object. The generated code includes:
 * - A lookup in the object's vTable if the method is virtual.
 * - A call to the method.
 *
 * @param reference_code The code representing the object reference.
 * @param method_name The name of the method to be called.
 * @param assumed_class The class to which the object is assumed to belong at compile-time.
 *
 * @return A code_segment structure containing the complete method call code:
 *         - pre_code: The setup code including the vTable lookup.
 *         - post_code: The code for cleaning up the vTable lookup.
 *         - code: The expression to call the method.
 */
code_segment generate_method_call_code(
	const std::string& reference_code,
	const std::string& method_name,
	std::shared_ptr<bpp::bpp_class> assumed_class,
	bool force_static_reference,
	std::shared_ptr<bpp::bpp_program> program
) {
	code_segment result;

	if (assumed_class == nullptr) {
		throw internal_error("Assumed class is null");
	}

	std::shared_ptr<bpp::bpp_method> assumed_method = assumed_class->get_method_UNSAFE(method_name);
	if (assumed_method == nullptr) {
		if (method_name.rfind("__", 0) == 0) {
			// If the method is a system method, we assume it exists
			// This is a hack to allow system methods to be called without being defined
			// TODO(@rail5): Replace this hack with proper system method registry
			assumed_method = std::make_shared<bpp::bpp_method>();
			assumed_method->set_name(method_name);
			assumed_method->set_scope(bpp_scope::SCOPE_PUBLIC);
			assumed_method->set_virtual(true);
		} else {
			throw internal_error("Method " + method_name + " not found in class " + assumed_class->get_name());
		}
	}

	// Is the method virtual?
	if (assumed_method->is_virtual() && !force_static_reference) {
		// Look up the method in the vTable
		result.pre_code = "if bpp____vTable__lookup \"" + reference_code + "\" \"" + method_name + "\" __func" + std::to_string(program->get_function_counter()) + "; then\n";
		result.post_code = "	unset __func" + std::to_string(program->get_function_counter()) + "\nfi\n";
		result.code = "	${!__func" + std::to_string(program->get_function_counter()) + "} " + reference_code;
		program->increment_function_counter();
	} else {
		result.code = "bpp__" + assumed_class->get_name() + "__" + method_name + " " + reference_code;
	}

	return result;
}

/**
 * @brief Generates a code segment for performing a dynamic cast.
 * 
 * This function constructs a code segment to perform a dynamic cast on an object. The generated code includes:
 * - A runtime check to verify the cast is valid.
 * - A substitution of either the address of the cast object or the nullptr value.
 * 
 * @param reference_code The code representing the object reference
 * @param class_name The type to which we want to cast
 * 
 * @return A code segment structure containing the complete dynamic cast code:
 * 	- pre_code: Call to the runtime dynamic cast function
 * 	- post_code: Code for cleaning up the dynamic cast's temporary variable
 * 	- code: The temporary variable containing the result of the dynamic cast (either the address or the @nullptr value)
 */
code_segment generate_dynamic_cast_code(
	const std::string& reference_code,
	const std::string& class_name,
	std::shared_ptr<bpp::bpp_program> program
) {
	code_segment result;

	result.pre_code = "bpp____dynamic__cast " + class_name + " __dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + " " + reference_code + "\n";
	result.code = "${__dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + "}";
	result.post_code = "unset __dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + "\n";

	program->increment_dynamic_cast_counter();

	return result;
}

/**
 * @brief Generates a code segment to INLINE class's "new" function within a method
 * 
 * This is only to be used inside a method.
 * 
 * Rather than calling the class's "new" function, by inlining it, we can ensure that all instantiated objects are purely local to the method.
 * This eliminates any issues with scope and recursion.
 * 
 * However, this change has resulted in some significant code duplication:
 * 		1. This function duplicates code in bpp_program::add_class,
 * 			with the difference that the created variables are declared 'local'
 * 		2. bpp_method now has an override for add_object which is identical to that in its parent class bpp_code_entity,
 * 			with the difference that rather than adding code which will call the class's "new" function at runtime,
 * 			it adds the code generated by *this* function.
 * 
 * This will have to be handled soon.
 * 
 * @param new_address The address of the new object
 * @param new_class The class of the new object
 * 
 * @return A code segment structure containing the complete new code:
 * 	- pre_code: All of the code necessary to create the object
 * 	- post_code: Empty
 * 	- code: Empty
 */
code_segment inline_new(
	const std::string&					new_address,
	std::shared_ptr<bpp::bpp_class>		new_class
) {
	code_segment result;
	result.pre_code = "eval \"local " + new_address + "____vPointer=bpp__" + new_class->get_name() + "____vTable\"";
	for (auto& dm : new_class->get_datamembers()) {
		result.pre_code += dm->get_pre_access_code() + "\n";
		if (dm->get_class()->get_name() == "primitive") {
			// Is it an array?
			if (dm->is_array()) {
				result.pre_code += "	eval \"local " + new_address + "__" + dm->get_name() + "=" + dm->get_default_value() + "\"\n";
			} else {
				result.pre_code += "	local __objAssignment=" + dm->get_default_value() + "\n";
				result.pre_code += "	eval \"local " + new_address + "__" + dm->get_name() + "=\\$__objAssignment\"\n";
				result.pre_code += "	unset __objAssignment\n";
			}
		} else if (dm->is_pointer()) {
			std::string default_value = dm->get_default_value();
			std::string default_value_preface = "";
			if (!default_value.empty() && default_value[0] == '$') {
				default_value_preface = "\\";
			}
			result.pre_code += "	eval \"local " + new_address + "__" + dm->get_name() + "=" + default_value_preface + default_value + "\"\n";
		} else {
			// Inline 'new' for the object
			result.pre_code += inline_new(new_address + "__" + dm->get_name(), dm->get_class()).pre_code;
			// Call the constructor if it exists
			if (dm->get_class()->get_method_UNSAFE("__constructor") != nullptr) {
				result.pre_code += "	bpp__" + dm->get_class()->get_name() + "____constructor \"" + new_address + "__" + dm->get_name() + "\"\n";
			}
		}
		result.pre_code += dm->get_post_access_code() + "\n";
	}

	return result;
}

/**
 * @brief Resolves a reference to an entity in a particular context.
 * 
 * This function resolves a reference to an entity (object, method, or data member) based on a sequence of identifiers.
 * 
 * It does this by recursively scanning the current context for the requested identifier,
 * and then setting a new context based on the resolution of that identifier.
 * I.e., for {object, method}, it will first scan the current context for 'object',
 * and then scan the 'object' context for 'method'.
 * 
 * If passing a self-reference (@this.... or @super....),
 * The self-referential keyword should be the first node in the identifiers deque.
 * 
 * @param context The context (code_entity) in which to resolve the reference.
 * @param identifiers A deque of identifiers making up the reference.
 * @param current_class The class (if any) in which the reference is being resolved.
 * @param program The program in which the reference is being resolved.
 * 
 * @return An entity_reference structure containing:
 *  - entity: A shared pointer to the resolved entity (object, method, or data member).
 *  - reference_code: A code_segment containing the code to access the entity.
 *  - created_first_temporary_variable: Whether a first temporary variable was necessary in the compiled code
 *  - created_second_temporary_variable: Whether a second temporary variable was necessary in the compiled code
 *  - class_containing_the_method: The class containing the method, if applicable.
 *  - error: An optional reference_error structure containing an error message and relevant token if the reference could not be resolved.
 */
entity_reference resolve_reference(
	std::shared_ptr<bpp::bpp_code_entity> context,
	std::deque<antlr4::tree::TerminalNode*> identifiers,
	std::shared_ptr<bpp::bpp_program> program
) {
	// TODO(@rail5): Track entity references by calling entity->add_reference(file, line, column) at each resolution step
	bool self_reference = identifiers.at(0)->getSymbol()->getType() == BashppLexer::KEYWORD_THIS
			|| identifiers.at(0)->getSymbol()->getType()            == BashppLexer::KEYWORD_THIS_LVALUE
			|| identifiers.at(0)->getSymbol()->getType()            == BashppLexer::KEYWORD_SUPER
			|| identifiers.at(0)->getSymbol()->getType()            == BashppLexer::KEYWORD_SUPER_LVALUE;
	
	bool super = identifiers.at(0)->getSymbol()->getType()          == BashppLexer::KEYWORD_SUPER
			|| identifiers.at(0)->getSymbol()->getType()            == BashppLexer::KEYWORD_SUPER_LVALUE;

	entity_reference result;

	std::shared_ptr<bpp::bpp_class> current_class = context->get_containing_class().lock();

	result.entity = context;
	if (self_reference) {
		result.entity = current_class;
	}

	bpp::reference_type last_reference_type = bpp::reference_type::ref_object;

	std::shared_ptr<bpp::bpp_object> object = nullptr;
	std::shared_ptr<bpp::bpp_datamember> datamember = nullptr;
	std::shared_ptr<bpp::bpp_method> method = nullptr;
	result.class_containing_the_method = current_class;

	if (!self_reference) {
		// Get the first object
		std::string first_object_name = identifiers.at(0)->getText();
		object = context->get_object(first_object_name);
		result.entity = object;

		if (object == nullptr) {
			result.error = entity_reference::reference_error{
				"Object not found: " + first_object_name,
				identifiers.at(0)
			};
			return result;
		}
	}

	if (super) {
		std::string derived_class_name = result.entity->get_name();
		result.entity = current_class->get_parent();
		if (result.entity == nullptr) {
			result.error = entity_reference::reference_error{
				derived_class_name + " has no parent class to reference with @super",
				identifiers.at(0)
			};
			return result;
		}
	}

	identifiers.pop_front();

	// The following two booleans are used for code generation
	// to determine how many layers of indirection are needed in generated code
	// E.g:
	// To access object.innerObject.innerMostObject.property
	// We need to resolve each step of the reference into its own shell variable at runtime
	// *After* we've created our first temporary variable, we need to start encasing
	// future references in ${...}
	// And *after* we've created our second temporary variable, we need to add
	// indirection with ${!...}
	result.created_first_temporary_variable = self_reference; // If it's a self-reference, the ${__this} pointer counts as our first temporary variable
	result.created_second_temporary_variable = false;

	if (object != nullptr && object->is_pointer()) {
		result.created_first_temporary_variable = true; // Having to dereference a pointer means creating a temporary variable
	}

	std::string encase_open = "";
	std::string encase_close = "";
	std::string indirection = "";

	if (self_reference) {
		result.reference_code.code = "__this";
	} else {
		result.reference_code.code = object->get_address();
	}

	for (auto& identifier : identifiers) {
		if (result.created_first_temporary_variable) {
			encase_open = "${";
			encase_close = "}";
		}
		if (result.created_second_temporary_variable) {
			indirection = "!";
		}

		switch (last_reference_type) {
			case bpp::reference_type::ref_object:
				// All OK, can descend the object hierarchy
				break;
			case bpp::reference_type::ref_primitive:
				result.error = entity_reference::reference_error{
					"Unexpected identifier after primitive object reference",
					identifier
				};
				return result;
			case bpp::reference_type::ref_method:
				result.error = entity_reference::reference_error{
					"Unexpected identifier after method reference",
					identifier
				};
				return result;
		}

		std::string identifier_text = identifier->getText();

		if (identifier_text.find("__") != std::string::npos) {
			result.error = entity_reference::reference_error{
				"Invalid identifier: " + identifier_text + "\nBash++ identifiers cannot contain double underscores",
				identifier
			};
			return result;
		}

		std::shared_ptr<bpp::bpp_class> reference_class = result.entity->get_class();

		object = nullptr;
		datamember = reference_class->get_datamember(identifier_text, current_class);
		method = reference_class->get_method(identifier_text, current_class);

		if (datamember == bpp::inaccessible_datamember || method == bpp::inaccessible_method) {
			result.error = entity_reference::reference_error{
				identifier_text + " is inaccessible in this context",
				identifier
			};
			return result;
		}

		if (method != nullptr) {
			result.class_containing_the_method = result.entity->get_class();
			last_reference_type = bpp::reference_type::ref_method;
			result.entity = method;
		} else if (datamember != nullptr) {
			last_reference_type = (datamember->get_class() == program->get_primitive_class())
					? bpp::reference_type::ref_primitive
					: bpp::reference_type::ref_object;
			result.entity = datamember;

			std::string temporary_variable_lvalue = result.reference_code.code + "__" + identifier_text;
			std::string temporary_varible_rvalue = "${" + indirection + result.reference_code.code + "}__" + identifier_text;

			if (result.created_first_temporary_variable) {
				result.reference_code.pre_code += temporary_variable_lvalue + "=" + temporary_varible_rvalue + "\n";
				result.reference_code.post_code += "unset " + temporary_variable_lvalue + "\n";
				result.created_second_temporary_variable = true;
			}

			result.reference_code.code = temporary_variable_lvalue;
			result.created_first_temporary_variable = true;
		} else {
			result.error = entity_reference::reference_error{
				result.entity->get_name() + " has no member named " + identifier_text,
				identifier
			};
			return result;
		}
	}

	// Having finished iterating over all the identifiers
	// And determining precisely which entity is referred to
	// We can now return
	return result;
}

} // namespace bpp
