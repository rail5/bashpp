/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterClass_definition(BashppParser::Class_definitionContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_class> new_class = std::make_shared<bpp::bpp_class>();
	entity_stack.push(new_class);

	// Get the class name
	std::string class_name = ctx->IDENTIFIER(0)->getText();

	// Verify that the class name is not already in use (or a protected keyword)
	if (is_protected_keyword(class_name)) {
		entity_stack.pop();
		throw_syntax_error(ctx->IDENTIFIER(0), "Invalid class name: " + class_name);
	}

	// Verify that the class name does not contain a double underscore
	if (class_name.find("__") != std::string::npos) {
		entity_stack.pop();
		throw_syntax_error(ctx->IDENTIFIER(0), "Invalid class name: " + class_name + "\nBash++ identifiers cannot contain double underscores");
	}

	if (program->get_class(class_name) != nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->IDENTIFIER(0), "Class already exists: " + class_name);
	}

	if (program->get_object(class_name) != nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->IDENTIFIER(0), "Object already exists: " + class_name);
	}

	new_class->set_name(class_name);
	program->prepare_class(new_class);

	// Inherit from a parent class if specified
	if (ctx->IDENTIFIER().size() > 1) {
		std::string parent_class_name = ctx->IDENTIFIER(1)->getText();
		std::shared_ptr<bpp::bpp_class> parent_class = program->get_class(parent_class_name);
		if (parent_class == nullptr) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(1), "Parent class not found: " + parent_class_name);
		}
		new_class->inherit(parent_class);
	}
}

void BashppListener::exitClass_definition(BashppParser::Class_definitionContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_class> new_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (new_class == nullptr) {
		throw internal_error("entity_stack top is not a bpp_class", ctx);
	}

	entity_stack.pop();

	new_class->finalize(program);

	// Add the class to the program
	program->add_class(new_class);
}
