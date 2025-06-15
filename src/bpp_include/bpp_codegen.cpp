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

	code_segment destructor_code = generate_method_call_code(object_ref, "__destructor", object->get_class(), program);
	result.pre_code += destructor_code.pre_code;
	result.pre_code += destructor_code.code + "\n";
	result.pre_code += destructor_code.post_code;

	code_segment delete_code = generate_method_call_code(object_ref, "__delete", object->get_class(), program);
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
	if (assumed_method->is_virtual()) {
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

	result.pre_code = "bpp____dynamic__cast " + reference_code + " " + class_name + " __dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + "\n";
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

} // namespace bpp
