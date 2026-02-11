/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterSupershell(std::shared_ptr<AST::Supershell> node) {
	/**
	 * Supershells take the form
	 * 	@(...)
	 * Where ... is a series of commands to be executed in a supershell
	 * Supershells can be nested
	 */
	
	in_supershell = true;

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Supershell outside of a code entity");
	}

	std::shared_ptr<bpp::bpp_string> supershell_entity = std::make_shared<bpp::bpp_string>();
	supershell_entity->set_containing_class(current_code_entity->get_containing_class());
	supershell_entity->inherit(current_code_entity);
	entity_stack.push(supershell_entity);

	supershell_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitSupershell(std::shared_ptr<AST::Supershell> node) {
	std::shared_ptr<bpp::bpp_string> supershell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (supershell_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Supershell context was not found in the entity stack");
	}

	entity_stack.pop();

	program->mark_entity(
		source_file,
		supershell_entity->get_initial_definition().line,
		supershell_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		supershell_entity
	);

	// Carry objects, classes, etc from the supershell to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity was not found in the entity stack");
	}
	current_code_entity->inherit(supershell_entity);

	std::string code_to_run;

	if (!supershell_entity->get_pre_code().empty()) {
		code_to_run = supershell_entity->get_pre_code() + "\n";
	}

	code_to_run += supershell_entity->get_code();

	if (!supershell_entity->get_post_code().empty()) {
		code_to_run += "\n" + supershell_entity->get_post_code();
	}

	code_segment supershell_code = generate_supershell_code(
		code_to_run,
		program);

	in_supershell = false;

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
