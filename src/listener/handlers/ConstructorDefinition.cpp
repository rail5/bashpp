/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node) {
	/**
	 * Constructor definitions take the form
	 * 	@constructor { ... }
	 */

	// Verify that we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor definition outside of class");
	}

	std::shared_ptr<bpp::bpp_method> constructor = std::make_shared<bpp::bpp_method>();
	constructor->set_name("__constructor");
	constructor->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
	constructor->set_containing_class(current_class);
	constructor->set_overridable(true); // Constructors are overridable, but not virtual
	constructor->inherit(program);
	entity_stack.push(constructor);

	constructor->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);

	// If this is a constructor for a derived class, and the parent class has a constructor, call it
	auto containing_class = constructor->get_containing_class().lock();
	if (containing_class == nullptr) {
		throw bpp::ErrorHandling::InternalError("Containing class not found for constructor");
	}
	auto parent_class = containing_class->get_parent();
	if (parent_class != nullptr) {
		auto parent_constructor = parent_class->get_method_UNSAFE("__constructor");
		if (parent_constructor != nullptr) {
			// Call the parent constructor
			code_segment parent_constructor_call = generate_constructor_call_code("${__this}", parent_class);
			constructor->add_code(parent_constructor_call.full_code() + "\n");
		}
	}
}

void BashppListener::exitConstructorDefinition(std::shared_ptr<AST::ConstructorDefinition> node) {
	std::shared_ptr<bpp::bpp_method> constructor = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	if (constructor == nullptr) {
		throw bpp::ErrorHandling::InternalError("Constructor definition not found on the entity stack");
	}

	entity_stack.pop();

	// Call destructors for any objects created in the constructor before we exit it
	constructor->destruct_local_objects(program);
	constructor->flush_code_buffers();

	// Add the constructor to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw bpp::ErrorHandling::InternalError("Class not found on the entity stack");
	}

	program->mark_entity(
		source_file,
		constructor->get_initial_definition().line,
		constructor->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		constructor
	);

	if (!current_class->add_method(constructor)) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Constructor already defined");
	}
}
