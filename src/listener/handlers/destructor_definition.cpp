/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_syntax_errors
	/**
	 * Destructor definitions take the form
	 * 	@destructor { ... }
	 */

	// Verify that we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw_syntax_error(ctx->KEYWORD_DESTRUCTOR(), "Destructor definition outside of class");
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
		ctx->KEYWORD_DESTRUCTOR()->getSymbol()->getLine() - 1,
		ctx->KEYWORD_DESTRUCTOR()->getSymbol()->getCharPositionInLine()
	);
}

void BashppListener::exitDestructor_definition(BashppParser::Destructor_definitionContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_method> destructor = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	if (destructor == nullptr) {
		throw internal_error("Destructor definition not found on the entity stack", ctx);
	}

	entity_stack.pop();

	// Call destructors for any objects created in the destructor before we exit it
	destructor->destruct_local_objects(program);

	// Add the destructor to the class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw internal_error("Class not found on the entity stack", ctx);
	}

	program->mark_entity(
		source_file,
		destructor->get_initial_definition().line,
		destructor->get_initial_definition().column,
		ctx->RBRACE()->getSymbol()->getLine() - 1,
		ctx->RBRACE()->getSymbol()->getCharPositionInLine(),
		destructor
	);

	if (!current_class->add_method(destructor)) {
		throw_syntax_error_from_exitRule(ctx->KEYWORD_DESTRUCTOR(), "Destructor already defined");
	}
}
