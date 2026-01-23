/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterBash53NativeSupershell(std::shared_ptr<AST::Bash53NativeSupershell> node) {
	skip_syntax_errors
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw_syntax_error(node, "Subshell substitution outside of code entity");
	}

	// Create a new code entity for the subshell
	std::shared_ptr<bpp::bpp_string> bash_53_native_supershell_entity = std::make_shared<bpp::bpp_string>();
	bash_53_native_supershell_entity->set_containing_class(code_entity->get_containing_class());
	bash_53_native_supershell_entity->inherit(code_entity);
	bash_53_native_supershell_entity->set_requires_perfect_forwarding(true);

	if (program->get_target_bash_version() < BashVersion{5, 3}) {
		show_warning(node,
			std::string("Bash 5.3 native supershells are not supported by the current Bash " + program->get_target_bash_version().to_string() + " target version. "
			+ "\nYou can change the target Bash version using the -b / --target-bash command-line option.")
		);
	}

	// Push the subshell entity onto the entity stack
	entity_stack.push(bash_53_native_supershell_entity);

	bash_53_native_supershell_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBash53NativeSupershell(std::shared_ptr<AST::Bash53NativeSupershell> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> bash_53_native_supershell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (bash_53_native_supershell_entity == nullptr) {
		throw internal_error("Subshell substitution context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	current_code_entity->add_code_to_previous_line(bash_53_native_supershell_entity->get_pre_code());
	current_code_entity->add_code_to_next_line("\n" + bash_53_native_supershell_entity->get_post_code());
	current_code_entity->add_code(node->STARTTOKEN().getValue() + bash_53_native_supershell_entity->get_code() + "}");

	program->mark_entity(
		source_file,
		bash_53_native_supershell_entity->get_initial_definition().line,
		bash_53_native_supershell_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		bash_53_native_supershell_entity
	);
}
