/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <listener/BashppListener.h>

void BashppListener::enterDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node) {
	/**
	 * Destructor definitions take the form
	 * 	@destructor { ... }
	 */

	// Verify that we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor definition outside of class");
	}

	std::shared_ptr<bpp::bpp_method> destructor = std::make_shared<bpp::bpp_method>();
	destructor->set_name("__destructor");
	destructor->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
	destructor->set_virtual(true);
	destructor->set_containing_class(current_class);
	destructor->inherit(program);
	entity_stack.push(destructor);

	destructor->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitDestructorDefinition(std::shared_ptr<AST::DestructorDefinition> node) {
	std::shared_ptr<bpp::bpp_method> destructor = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	if (destructor == nullptr) {
		throw bpp::ErrorHandling::InternalError("Destructor definition not found on the entity stack");
	}

	entity_stack.pop();

	// Call destructors for any objects created in the destructor before we exit it
	destructor->destruct_local_objects(program);

	// If this is a destructor for a derived class, and the parent class has a destructor, call it
	auto containing_class = destructor->get_containing_class().lock();
	if (containing_class == nullptr) {
		throw bpp::ErrorHandling::InternalError("Containing class not found for destructor");
	}
	auto parent_class = containing_class->get_parent();
	if (parent_class != nullptr) {
		auto parent_destructor = parent_class->get_method_UNSAFE("__destructor");
		if (parent_destructor != nullptr) {
			// Call the parent destructor
			code_segment parent_destructor_call = generate_destructor_call_code("${__this}", parent_class, true, program);
			destructor->add_code(parent_destructor_call.full_code() + "\n");
		}
	}

	destructor->flush_code_buffers();

	// Add the destructor to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw bpp::ErrorHandling::InternalError("Class not found on the entity stack");
	}

	program->mark_entity(
		source_file,
		destructor->get_initial_definition().line,
		destructor->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		destructor
	);

	if (!current_class->add_method(destructor)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Destructor already defined");
	}
}
