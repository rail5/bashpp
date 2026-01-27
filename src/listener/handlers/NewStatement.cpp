/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterNewStatement(std::shared_ptr<AST::NewStatement> node) {
	/**
	 * New statements take the form
	 * 	@new ClassName
	 * Where ClassName is the name of the class to instantiate
	 * 
	 * This statement creates a new object of the specified class
	 * And replaces the "@new ClassName" statement with the address of the new object
	*/

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "New statement outside of code entity");
	}

	// Verify that the class actually exists
	std::string class_name = node->TYPE().getValue();
	std::shared_ptr<bpp::bpp_class> new_class = current_code_entity->get_class(class_name);

	if (new_class == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node->TYPE(), "Class not found: " + class_name);
	}

	new_class->add_reference(
		source_file,
		node->TYPE().getLine(),
		node->TYPE().getCharPositionInLine()
	);

	// Call the class's "new" method in a supershell and substitute the result
	std::string new_method_call = "bpp__" + class_name + "____new";

	code_segment new_code = generate_supershell_code(new_method_call, program);
	current_code_entity->add_code_to_previous_line(new_code.pre_code);

	// Create a temporary variable to hold the output of the supershell
	std::string tmp_storage_var = "__newAssignment" + std::to_string(program->get_assignment_counter());
	current_code_entity->add_code_to_previous_line(tmp_storage_var + "=" + new_code.code + "\n");
	current_code_entity->add_code_to_next_line("unset " + tmp_storage_var + "\n");
	program->increment_assignment_counter();

	// Call the constructor if it exists
	auto constructor_method = new_class->get_method_UNSAFE("__constructor");
	if (constructor_method != nullptr) {
		// The 'new' function was called in a supershell, and its output was stored in the variable given in new_code.code
		// This output is the pointer to the new object
		// Call the constructor with this pointer as the argument
		auto constructor_code = generate_constructor_call_code(tmp_storage_var, new_class);
		current_code_entity->add_code_to_previous_line(constructor_code.full_code());
	}

	current_code_entity->add_code_to_next_line(new_code.post_code);
	current_code_entity->add_code("${" + tmp_storage_var + "}");
}

void BashppListener::exitNewStatement(std::shared_ptr<AST::NewStatement> node) {}
