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
	std::shared_ptr<bpp::bpp_program> program
) {
	code_segment result;

	uint64_t supershell_counter = program->get_supershell_counter();

	std::string supershell_function_name = "____supershellRunFunc" + std::to_string(supershell_counter);
	std::string supershell_output_variable = "____supershellOutput" + std::to_string(supershell_counter);

	result.pre_code += "function " + supershell_function_name + "() {\n";
	result.pre_code += "	" + code_to_run + "\n";
	result.pre_code += "}\n";

	// Supershells were introduced as a native form of command substitution in Bash 5.3
	// If we're targeting Bash 5.3 or later, we can just use the native implementation
	// This is more efficient and avoids the need for a custom supershell function
	// Otherwise, run our old logic for Bash++ supershells
	auto target_bash_version = program->get_target_bash_version();
	
	if (target_bash_version >= BashVersion{5, 3}) {
		result.code = "${ " + supershell_function_name + "; }";
		program->increment_supershell_counter();
		return result;
	}

	// If we haven't returned yet, we're targeting Bash 5.2 or earlier
	// Carry on

	result.pre_code += "bpp____supershell " + supershell_output_variable + " " + supershell_function_name + "\n";
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

	code_segment destructor_code = generate_destructor_call_code(object_ref, object->get_class(), false, program);
	result.pre_code += destructor_code.full_code() + "\n";

	code_segment delete_code = generate_method_call_code(object_ref, "__delete", object->get_class(), false, program);
	result.pre_code += delete_code.full_code() + "\n";

	return result;
}

inline code_segment _generate_virtual_method_call_code(
	const std::string& reference_code,
	const std::string& method_name,
	std::shared_ptr<bpp::bpp_program> program
) {
	code_segment result;

	// Perform a vTable lookup, store the result in the temporary variable, and execute
	result.pre_code = "if bpp____vTable__lookup \"" + reference_code + "\" \"" + method_name + "\" __func" + std::to_string(program->get_function_counter()) + "; then\n";
	result.post_code = "	unset __func" + std::to_string(program->get_function_counter()) + "\nfi\n";
	result.code = "	${!__func" + std::to_string(program->get_function_counter()) + "} " + reference_code;
	program->increment_function_counter();

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
		throw bpp::ErrorHandling::InternalError("Assumed class is null");
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
			throw bpp::ErrorHandling::InternalError("Method " + method_name + " not found in class " + assumed_class->get_name());
		}
	}

	// Is the method virtual?
	if (assumed_method->is_virtual() && !force_static_reference) {
		return _generate_virtual_method_call_code(reference_code, method_name, program);
	} else {
		std::string class_name = assumed_class->get_name();
		auto class_containing_the_method = assumed_method->get_containing_class().lock();
		if (class_containing_the_method != nullptr) {
			class_name = class_containing_the_method->get_name();
		}
		result.code = "bpp__" + class_name + "__" + method_name + " " + reference_code;
	}

	return result;
}

code_segment generate_constructor_call_code(
	const std::string& reference_code,
	std::shared_ptr<bpp_class> assumed_class
) {
	code_segment result;
	auto constructor_method = assumed_class->get_method_UNSAFE("__constructor");
	if (constructor_method == nullptr) return result;

	std::string class_name = assumed_class->get_name();
	auto class_containing_the_constructor = constructor_method->get_containing_class().lock();
	if (class_containing_the_constructor != nullptr) {
		class_name = class_containing_the_constructor->get_name();
	}

	result.code = "bpp__" + class_name + "____constructor " + reference_code + "\n";

	return result;
}

code_segment generate_destructor_call_code(
	const std::string& reference_code,
	std::shared_ptr<bpp_class> assumed_class,
	bool force_static_reference,
	std::shared_ptr<bpp::bpp_program> program
) {
	auto destructor_method = assumed_class->get_method_UNSAFE("__destructor");
	if (destructor_method == nullptr) return code_segment();

	// All destructors are virtual
	if (!force_static_reference) {
		return _generate_virtual_method_call_code(reference_code, "__destructor", program);
	} else {
		std::string class_name = assumed_class->get_name();
		auto class_containing_the_destructor = destructor_method->get_containing_class().lock();
		if (class_containing_the_destructor != nullptr) {
			class_name = class_containing_the_destructor->get_name();
		}
		code_segment result;
		result.code = "bpp__" + class_name + "____destructor " + reference_code;
		return result;
	}
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

	result.pre_code = "bpp____dynamic__cast \"" + class_name + "\" \"__dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + "\" \"" + reference_code + "\"\n";
	result.code = "${__dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + "}";
	result.post_code = "unset __dynamicCast" + std::to_string(program->get_dynamic_cast_counter()) + "\n";

	program->increment_dynamic_cast_counter();

	return result;
}

code_segment generate_typeof_code(
	const std::string& reference_code,
	std::shared_ptr<bpp::bpp_program> program
) {
	code_segment result;

	result.pre_code = "bpp____typeof " + reference_code + " __typeof" + std::to_string(program->get_typeof_counter()) + "\n";
	result.code = "${__typeof" + std::to_string(program->get_typeof_counter()) + "}";
	result.post_code = "unset __typeof" + std::to_string(program->get_typeof_counter()) + "\n";

	program->increment_typeof_counter();

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
				auto constructor_code = generate_constructor_call_code(new_address + "__" + dm->get_name(), dm->get_class());
				result.pre_code += constructor_code.full_code();
			}
		}
		result.pre_code += dm->get_post_access_code() + "\n";
	}

	return result;
}

/**
 * @brief Encases a temporary variable reference with the appropriate level of indirection
 *
 * This function takes a std::string which is the name of a shell variable we would like to use,
 * (for example, a temporary variable created during reference resolution),
 * and an indirection level (0, 1, or 2),
 * and returns the variable name encased in the appropriate amount of indirection.
 *
 * 0: Returns 'var' (as-is, no encasing)
 * 1: Returns '${var}'
 * 2: Returns '${!var}'
 *
 * Any other indirection level is treated as 0.
 * 
 * @param ref The reference string
 * @param indirection_level The level of indirection (0, 1, or 2)
 * @return std::string The encased reference string
 */
std::string get_encased_ref(const std::string& ref, uint8_t indirection_level) {
	std::string encased;
	std::string encase_open, encase_close, indirection;
	switch (indirection_level) {
		case 2:
			indirection = "!";
			[[ fallthrough ]];
		case 1:
			encase_open = "${";
			encase_close = "}";
			[[ fallthrough ]];
		default:
			[[ fallthrough ]];
		case 0:
			// No encasing
			break;
	}
	encased = encase_open + indirection + ref + encase_close;
	return encased;
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
 * @param file The source file in which the reference is being resolved.
 * @param context The context (code_entity) in which to resolve the reference.
 * @param nodes A deque of TerminalNode pointers representing the identifiers in the reference.
 * @param identifiers A deque of strings representing the identifiers in the reference.
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
entity_reference resolve_reference_impl(
	const std::string& file,
	std::shared_ptr<bpp::bpp_entity> context,
	std::deque<AST::Token<std::string>>* nodes,
	std::deque<std::string>* identifiers,
	bool declare_local,
	std::shared_ptr<bpp::bpp_program> program
) {
	std::deque<AST::Token<std::string>> nds = static_cast<std::deque<AST::Token<std::string>>>(*nodes);
	std::deque<std::string> ids = static_cast<std::deque<std::string>>(*identifiers);

	bool self_reference = ids.front() == "this" || ids.front() == "super";
	bool super = ids.front() == "super";

	// This function can be called with either:
	// A deque of TerminalNode pointers, or
	// A deque of strings representing identifiers.
	// All we need to resolve the reference is the set of strings,
	// But if we're also given the TerminalNode pointers,
	// Then we can (a) provide error messages (necessary in compilation),
	// And (b) keep track of where each entity is referenced in the source code.
	// If this is a request from the language server,
	// Ie, after all the analysis has already been done,
	// Then a deque of strings is perfectly fine -- we don't need to track positions.
	AST::Token<std::string> error_token;

	entity_reference result;

	std::shared_ptr<bpp::bpp_class> current_class = context->get_containing_class().lock();

	result.entity = context;
	if (self_reference) {
		result.entity = current_class;
	}

	std::string temporary_variable_declaration_prefix = declare_local ? "local " : "";

	bpp::reference_type last_reference_type = bpp::reference_type::ref_object;

	std::shared_ptr<bpp::bpp_object> object = nullptr;
	std::shared_ptr<bpp::bpp_datamember> datamember = nullptr;
	std::shared_ptr<bpp::bpp_method> method = nullptr;
	result.class_containing_the_method = current_class;

	if (!self_reference) {
		// Get the first object
		std::string first_object_name = ids.front();
		object = context->get_object(first_object_name);
		result.entity = object;

		if (object == nullptr) {
			if (!nds.empty()) {
				error_token = nds.at(0);
			}
			result.error = entity_reference::reference_error{
				"Object not found: " + first_object_name,
				error_token
			};
			return result;
		}

		if (!nds.empty()) {
			object->add_reference(
				file,
				nds.front().getLine(),
				nds.front().getCharPositionInLine()
			);
		}
	}

	if (super) {
		std::string derived_class_name = result.entity->get_name();
		result.entity = current_class->get_parent();
		current_class = current_class->get_parent();
		if (result.entity == nullptr) {
			if (!nds.empty()) {
				error_token = nds.at(0);
			}
			result.error = entity_reference::reference_error{
				derived_class_name + " has no parent class to reference with @super",
				error_token
			};
			return result;
		}
	}

	ids.pop_front();

	if (!nds.empty()) {
		nds.pop_front();
	}

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

	if (self_reference) {
		result.reference_code.code = "__this";
	} else {
		result.reference_code.code = object->get_address();
	}

	while (!ids.empty()) {
		if (!nds.empty()) error_token = nds.front();

		uint8_t indirection_level = 0;
		if (result.created_first_temporary_variable) indirection_level++;
		if (result.created_second_temporary_variable) indirection_level++;

		switch (last_reference_type) {
			case bpp::reference_type::ref_object:
				// All OK, can descend the object hierarchy
				break;
			case bpp::reference_type::ref_primitive:
				result.error = entity_reference::reference_error{
					"Unexpected identifier after primitive object reference",
					error_token
				};
				return result;
			case bpp::reference_type::ref_method:
				result.error = entity_reference::reference_error{
					"Unexpected identifier after method reference",
					error_token
				};
				return result;
		}

		if (ids.front().find("__") != std::string::npos) {
			result.error = entity_reference::reference_error{
				"Invalid identifier: " + ids.front() + "\nBash++ identifiers cannot contain double underscores",
				error_token
			};
			return result;
		}

		std::shared_ptr<bpp::bpp_class> reference_class = result.entity->get_class();

		object = nullptr;
		datamember = reference_class->get_datamember(ids.front(), current_class);
		method = reference_class->get_method(ids.front(), current_class);

		if (datamember == bpp::inaccessible_datamember || method == bpp::inaccessible_method) {
			result.error = entity_reference::reference_error{
				ids.front() + " is inaccessible in this context",
				error_token
			};
			return result;
		}

		if (method != nullptr) {
			result.class_containing_the_method = result.entity->get_class();
			last_reference_type = bpp::reference_type::ref_method;
			result.entity = method;

			if (!nds.empty()) {
				method->add_reference(
					file,
					nds.front().getLine(),
					nds.front().getCharPositionInLine()
				);
			}
		} else if (datamember != nullptr) {
			last_reference_type = (datamember->get_class() == program->get_primitive_class())
					? bpp::reference_type::ref_primitive
					: bpp::reference_type::ref_object;
			result.entity = datamember;

			std::string temporary_variable_lvalue = result.reference_code.code + "__" + ids.front();
			std::string temporary_variable_rvalue = get_encased_ref(result.reference_code.code, indirection_level) + "__" + ids.front();

			if (result.created_first_temporary_variable) {
				result.reference_code.pre_code += 
					temporary_variable_declaration_prefix + temporary_variable_lvalue + "=" + temporary_variable_rvalue + "\n";
				result.reference_code.post_code += "unset " + temporary_variable_lvalue + "\n";
				result.created_second_temporary_variable = true;
			}

			result.reference_code.code = temporary_variable_lvalue;
			result.created_first_temporary_variable = true;

			if (!nds.empty()) {
				datamember->add_reference(
					file,
					nds.front().getLine(),
					nds.front().getCharPositionInLine()
				);
			}
		} else {
			result.error = entity_reference::reference_error{
				result.entity->get_name() + " has no member named " + ids.front(),
				error_token
			};
			return result;
		}

		ids.pop_front();

		if (!nds.empty()) {
			nds.pop_front();
		}
	}

	// Having finished iterating over all the identifiers
	// And determining precisely which entity is referred to
	// We can now return
	return result;
}

} // namespace bpp
