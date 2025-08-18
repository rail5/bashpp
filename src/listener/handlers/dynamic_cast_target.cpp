/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterDynamic_cast_target(BashppParser::Dynamic_cast_targetContext *ctx) {
	skip_syntax_errors
	/**
	 * Dynamic cast targets are the part of a dynamic cast statement that specifies the class we're casting to
	 * This can be either an identifier (the name of the class), a shell variable (the name of the class is stored in the variable),
	 * or an object reference (the class is determined by the output of the object reference)
	 */

	std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(entity_stack.top());
	if (dynamic_cast_entity == nullptr) {
		throw internal_error("Dynamic cast context was not found in the entity stack", ctx);
	}

	std::shared_ptr<bpp::bpp_string> cast_target_entity = std::make_shared<bpp::bpp_string>();
	cast_target_entity->set_containing_class(dynamic_cast_entity->get_containing_class());
	cast_target_entity->inherit(dynamic_cast_entity);
	entity_stack.push(cast_target_entity);
}

void BashppListener::exitDynamic_cast_target(BashppParser::Dynamic_cast_targetContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> cast_target_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	entity_stack.pop();
	if (cast_target_entity == nullptr) {
		throw internal_error("Dynamic cast target context was not found in the entity stack", ctx);
	}

	std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(entity_stack.top());
	if (dynamic_cast_entity == nullptr) {
		throw internal_error("Dynamic cast context was not found in the entity stack", ctx);
	}

	// Which kind of input did we receive?
	if (ctx->IDENTIFIER() != nullptr) {
		// We have an identifier, so this is a class name
		std::string class_name = ctx->IDENTIFIER()->getText();
		dynamic_cast_entity->set_cast_to(class_name);

		// Check if the class exists
		std::shared_ptr<bpp::bpp_class> cast_class = dynamic_cast_entity->get_class(class_name);
		if (cast_class == nullptr) {
			show_warning(ctx->IDENTIFIER(), "Class not found: " + class_name + ". This cast may fail at runtime.");
		} else {
			cast_class->add_reference(
				source_file,
				ctx->IDENTIFIER()->getSymbol()->getLine() - 1,
				ctx->IDENTIFIER()->getSymbol()->getCharPositionInLine()
			);
		}
	} else if (ctx->BASH_VAR() != nullptr) {
		// The cast-to class is stored in a shell variable
		// Which will have to be evaluated at runtime
		dynamic_cast_entity->set_cast_to(ctx->BASH_VAR()->getText());
	} else {
		// There was an intervening object reference, which was handled by its respective parser rule,
		// And the resolved reference will have been added to this target entity via ->add_code()
		dynamic_cast_entity->set_cast_to(cast_target_entity->get_code());
		dynamic_cast_entity->add_code_to_previous_line(cast_target_entity->get_pre_code());
		dynamic_cast_entity->add_code_to_next_line(cast_target_entity->get_post_code());
	}
}
