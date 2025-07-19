/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterSupershell(BashppParser::SupershellContext *ctx) {
	skip_syntax_errors
	/**
	 * Supershells take the form
	 * 	@(...)
	 * Where ... is a series of commands to be executed in a supershell
	 * Supershells can be nested
	 */

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->SUPERSHELL_START(), "Supershell outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> supershell_entity = std::make_shared<bpp::bpp_string>();
	supershell_entity->set_containing_class(current_code_entity->get_containing_class());
	supershell_entity->inherit(current_code_entity);
	entity_stack.push(supershell_entity);

	supershell_entity->set_definition_position(
		source_file,
		ctx->SUPERSHELL_START()->getSymbol()->getLine() - 1,
		ctx->SUPERSHELL_START()->getSymbol()->getCharPositionInLine()
	);
}

void BashppListener::exitSupershell(BashppParser::SupershellContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> supershell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (supershell_entity == nullptr) {
		throw internal_error("Supershell context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	program->mark_entity(
		source_file,
		supershell_entity->get_initial_definition().line,
		supershell_entity->get_initial_definition().column,
		ctx->SUPERSHELL_END()->getSymbol()->getLine() - 1,
		ctx->SUPERSHELL_END()->getSymbol()->getCharPositionInLine(),
		supershell_entity
	);

	// Carry objects, classes, etc from the supershell to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity was not found in the entity stack", ctx);
	}
	current_code_entity->inherit(supershell_entity);

	code_segment supershell_code = generate_supershell_code(supershell_entity->get_pre_code() + supershell_entity->get_code() + "\n" + supershell_entity->get_post_code(), in_while_condition, current_while_condition, program);

	// If we're in an assignment, add the supershell code to the assignment
	std::shared_ptr<bpp::bpp_object_assignment> object_assignment = std::dynamic_pointer_cast<bpp::bpp_object_assignment>(current_code_entity);
	if (object_assignment != nullptr) {
		object_assignment->add_code_to_previous_line(supershell_code.pre_code);
		object_assignment->add_code_to_next_line(supershell_code.post_code);
		object_assignment->set_rvalue(supershell_code.code);
		return;
	}

	// If we're not in any broader context, simply add the supershell code to the current code entity
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(supershell_code.pre_code);
		current_code_entity->add_code_to_next_line(supershell_code.post_code);
		current_code_entity->add_code(supershell_code.code);
		return;
	}
}
