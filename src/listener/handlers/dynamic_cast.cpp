/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterDynamic_cast_statement(BashppParser::Dynamic_cast_statementContext *ctx) {
	skip_syntax_errors
	/**
	 * Dynamic cast statements take the form
	 * 	@dynamic_cast<ClassName> Object
	 * Where ClassName is the name of the class to cast to
	 * And Object is the object to cast
	 * 
	 * This statement performs a runtime check to verify the cast is valid
	 * And substitutes either the address of the cast object or the @nullptr value
	*/

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->KEYWORD_DYNAMIC_CAST(), "Dynamic cast statement outside of code entity");
	}

	std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::make_shared<bpp::bpp_dynamic_cast_statement>();
	dynamic_cast_entity->set_containing_class(current_code_entity->get_containing_class());
	dynamic_cast_entity->inherit(current_code_entity);

	// Verify that the class actually exists
	std::string class_name = ctx->IDENTIFIER()->getText();
	std::shared_ptr<bpp::bpp_class> cast_class = current_code_entity->get_class(class_name);

	if (cast_class == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(), "Class not found: " + class_name);
	}

	dynamic_cast_entity->set_cast_to(cast_class);
	entity_stack.push(dynamic_cast_entity);
}

void BashppListener::exitDynamic_cast_statement(BashppParser::Dynamic_cast_statementContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_dynamic_cast_statement> dynamic_cast_entity = std::dynamic_pointer_cast<bpp::bpp_dynamic_cast_statement>(entity_stack.top());
	if (dynamic_cast_entity == nullptr) {
		throw internal_error("Dynamic cast context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack", ctx);
	}

	code_segment dynamic_cast_code = generate_dynamic_cast_code(dynamic_cast_entity->get_code(), dynamic_cast_entity->get_cast_to()->get_name(), program);

	current_code_entity->add_code_to_previous_line(dynamic_cast_entity->get_pre_code());
	current_code_entity->add_code_to_previous_line(dynamic_cast_code.pre_code);

	current_code_entity->add_code_to_next_line(dynamic_cast_entity->get_post_code());
	current_code_entity->add_code_to_next_line(dynamic_cast_code.post_code);

	current_code_entity->add_code(dynamic_cast_code.code);
}
