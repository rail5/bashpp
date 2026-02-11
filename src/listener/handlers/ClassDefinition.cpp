/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterClassDefinition(std::shared_ptr<AST::ClassDefinition> node) {
	std::shared_ptr<bpp::bpp_class> new_class = std::make_shared<bpp::bpp_class>();
	new_class->inherit(program);
	entity_stack.push(new_class);

	in_class = true;

	new_class->set_definition_position(
		source_file,
		node->CLASSNAME().getLine(),
		node->CLASSNAME().getCharPositionInLine()
	);

	// Get the class name
	std::string class_name = node->CLASSNAME().getValue();

	// Verify that the class name is valid
	if (!bpp::is_valid_identifier(class_name)) {
		entity_stack.pop();
		// If, specifically, it contains a double underscore, we can provide a more specific error message
		if (class_name.find("__") != std::string::npos) {
			throw bpp::ErrorHandling::SyntaxError(this, node->CLASSNAME(), "Invalid class name: " + class_name + "\nBash++ identifiers cannot contain double underscores");
		} else {
			throw bpp::ErrorHandling::SyntaxError(this, node->CLASSNAME(), "Invalid class name: " + class_name);
		}
	}

	// Verify that the class name is not already in use
	if (program->get_class(class_name) != nullptr) {
		entity_stack.pop();
		throw bpp::ErrorHandling::SyntaxError(this, node->CLASSNAME(), "Class already exists: " + class_name);
	}

	if (program->get_object(class_name) != nullptr) {
		entity_stack.pop();
		throw bpp::ErrorHandling::SyntaxError(this, node->CLASSNAME(), "Object already exists: " + class_name);
	}

	new_class->set_name(class_name);
	program->prepare_class(new_class);

	// Inherit from a parent class if specified
	if (node->PARENTCLASSNAME().has_value()) {
		auto parent_class_node = node->PARENTCLASSNAME().value();
		std::string parent_class_name = parent_class_node.getValue();
		std::shared_ptr<bpp::bpp_class> parent_class = program->get_class(parent_class_name);
		if (parent_class == nullptr) {
			entity_stack.pop();
			throw bpp::ErrorHandling::SyntaxError(this, node->PARENTCLASSNAME().value(), "Parent class not found: " + parent_class_name);
		}
		new_class->inherit(parent_class);

		parent_class->add_reference(
			source_file,
			parent_class_node.getLine(),
			parent_class_node.getCharPositionInLine()
		);
	}
}

void BashppListener::exitClassDefinition(std::shared_ptr<AST::ClassDefinition> node) {
	std::shared_ptr<bpp::bpp_class> new_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (new_class == nullptr) {
		throw bpp::ErrorHandling::InternalError("entity_stack top is not a bpp_class");
	}

	entity_stack.pop();

	auto copy_method = bpp::generate_copy_method(new_class, program);
	new_class->add_method(copy_method);

	auto new_method = bpp::generate_new_method(new_class);
	new_class->add_method(new_method);

	auto delete_method = bpp::generate_delete_method(new_class);
	new_class->add_method(delete_method);

	// Add the class to the program
	program->add_class(new_class);

	// Mark the class's position in the file's entity map
	// This is important for the LSP to know which entities are active at which points in the file
	// This is used for features such as "Go to Definition" and "Find References"
	program->mark_entity(
		source_file,
		new_class->get_initial_definition().line,
		new_class->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		new_class
	);

	in_class = false;
}
