/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterClassDefinition(std::shared_ptr<AST::ClassDefinition> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_class> new_class = std::make_shared<bpp::bpp_class>();
	new_class->inherit(program);
	entity_stack.push(new_class);

	in_class = true;

	new_class->set_definition_position(
		source_file,
		node->getLine() - 1,
		node->getCharPositionInLine()
	);

	// Get the class name
	std::string class_name = node->CLASSNAME().getValue();

	// Verify that the class name is valid
	if (!bpp::is_valid_identifier(class_name)) {
		entity_stack.pop();
		// If, specifically, it contains a double underscore, we can provide a more specific error message
		if (class_name.find("__") != std::string::npos) {
			throw_syntax_error(node->CLASSNAME(), "Invalid class name: " + class_name + "\nBash++ identifiers cannot contain double underscores");
		} else {
			throw_syntax_error(node->CLASSNAME(), "Invalid class name: " + class_name);
		}
	}

	// Verify that the class name is not already in use
	if (program->get_class(class_name) != nullptr) {
		entity_stack.pop();
		throw_syntax_error(node->CLASSNAME(), "Class already exists: " + class_name);
	}

	if (program->get_object(class_name) != nullptr) {
		entity_stack.pop();
		throw_syntax_error(node->CLASSNAME(), "Object already exists: " + class_name);
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
			throw_syntax_error(node->PARENTCLASSNAME().value(), "Parent class not found: " + parent_class_name);
		}
		new_class->inherit(parent_class);

		parent_class->add_reference(
			source_file,
			parent_class_node.getLine() - 1,
			parent_class_node.getCharPositionInLine()
		);
	}
}

void BashppListener::exitClassDefinition(std::shared_ptr<AST::ClassDefinition> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_class> new_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (new_class == nullptr) {
		throw internal_error("entity_stack top is not a bpp_class");
	}

	entity_stack.pop();

	new_class->finalize(program);

	// Add the class to the program
	program->add_class(new_class);

	// Mark the class's position in the file's entity map
	// This is important for the LSP to know which entities are active at which points in the file
	// This is used for features such as "Go to Definition" and "Find References"
	program->mark_entity(
		source_file,
		new_class->get_initial_definition().line,
		new_class->get_initial_definition().column,
		node->getEndPosition().line - 1,
		node->getEndPosition().column,
		new_class
	);

	in_class = false;
}
