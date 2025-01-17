/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_CLASS_DEFINITION_CPP_
#define ANTLR_LISTENER_HANDLERS_CLASS_DEFINITION_CPP_

#include "../BashppListener.h"

void BashppListener::enterClass_definition(BashppParser::Class_definitionContext *ctx) {
	skip_comment
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_class> new_class = std::make_shared<bpp::bpp_class>();
	entity_stack.push(new_class);

	// Get the class name
	std::string class_name = ctx->IDENTIFIER(0)->getText();

	// Verify that the class name is not already in use (or a protected keyword)
	if (protected_keywords.find(class_name) != protected_keywords.end()) {
		throw_syntax_error(ctx->IDENTIFIER(0), "Invalid class name: " + class_name);
	}

	if (program->get_class(class_name) != nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(0), "Class already exists: " + class_name);
	}

	if (program->get_object(class_name) != nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(0), "Object already exists: " + class_name);
	}

	new_class->set_name(class_name);

	// Inherit from a parent class if specified
	if (ctx->IDENTIFIER().size() > 1) {
		std::string parent_class_name = ctx->IDENTIFIER(1)->getText();
		std::shared_ptr<bpp::bpp_class> parent_class = program->get_class(parent_class_name);
		if (parent_class == nullptr) {
			throw_syntax_error(ctx->IDENTIFIER(1), "Parent class not found: " + parent_class_name);
		}
		new_class->inherit(parent_class);
	}
}

void BashppListener::exitClass_definition(BashppParser::Class_definitionContext *ctx) {
	skip_comment
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_class> new_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (new_class == nullptr) {
		throw internal_error("entity_stack top is not a bpp_class");
	}

	entity_stack.pop();

	// Add the class to the program
	program->add_class(new_class);
}

#endif // ANTLR_LISTENER_HANDLERS_CLASS_DEFINITION_CPP_
