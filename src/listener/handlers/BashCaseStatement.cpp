/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

/**
* This file contains parser rules for all of:
* 		1. BashCaseStatement
* 		2. BashCasePattern
* 		3. BashCasePatternHeader
*/

#include <listener/BashppListener.h>

void BashppListener::enterBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node) {
	/**
	 * Bash case statements take the form
	 * 		case (something) in
	 * 			pattern1)
	 * 				...
	 * 				;;
	 * 			...
	 * 		esac
	 */

	 std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	 if (current_code_entity == nullptr) {
		 throw bpp::ErrorHandling::SyntaxError(this, node, "Case statement outside of code entity");
	 }

	 std::shared_ptr<bpp::bash_case> case_statement_entity = std::make_shared<bpp::bash_case>();
	 case_statement_entity->set_containing_class(current_code_entity->get_containing_class());
	 case_statement_entity->inherit(current_code_entity);

	 entity_stack.push(case_statement_entity);
}

void BashppListener::exitBashCaseStatement(std::shared_ptr<AST::BashCaseStatement> node) {
	std::shared_ptr<bpp::bash_case> case_statement_entity = std::dynamic_pointer_cast<bpp::bash_case>(entity_stack.top());

	if (case_statement_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case statement entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Current code entity not found in the entity stack");
	}

	current_code_entity->add_code_to_previous_line(case_statement_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(case_statement_entity->get_post_code());
	current_code_entity->add_code("case " + case_statement_entity->get_code() + " in\n" + case_statement_entity->get_cases() + "\nesac\n");
}

// It's not necessary to handle BashCaseInput nodes specially.
// Their inner nodes will simply call ->add_code on the containing case statement entity,
// Which will correctly place their content as the input to the case statement.

void BashppListener::enterBashCasePattern(std::shared_ptr<AST::BashCasePattern> node) {
	std::shared_ptr<bpp::bash_case> case_statement_entity = std::dynamic_pointer_cast<bpp::bash_case>(entity_stack.top());

	if (case_statement_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case statement entity not found in the entity stack");
	}

	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::make_shared<bpp::bash_case_pattern>();
	case_pattern_entity->set_containing_class(case_statement_entity->get_containing_class());
	case_pattern_entity->inherit(case_statement_entity);

	case_pattern_entity->set_containing_case(case_statement_entity);

	entity_stack.push(case_pattern_entity);

	case_pattern_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitBashCasePattern(std::shared_ptr<AST::BashCasePattern> node) {
	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::dynamic_pointer_cast<bpp::bash_case_pattern>(entity_stack.top());

	if (case_pattern_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case pattern entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_case> case_statement_entity = std::dynamic_pointer_cast<bpp::bash_case>(entity_stack.top());

	if (case_statement_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case statement entity not found in the entity stack");
	}

	case_statement_entity->add_case(case_pattern_entity->get_pattern() + ")\n"
		+ case_pattern_entity->get_pre_code() + "\n"
		+ case_pattern_entity->get_code() + "\n"
		+ case_pattern_entity->get_post_code()
		+ "\n;;\n");
	
	program->mark_entity(
		source_file,
		case_pattern_entity->get_initial_definition().line,
		case_pattern_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		case_pattern_entity
	);
}

void BashppListener::enterBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node) {
	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::dynamic_pointer_cast<bpp::bash_case_pattern>(entity_stack.top());

	if (case_pattern_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case pattern entity not found in the entity stack");
	}

	std::shared_ptr<bpp::bpp_string> case_pattern_header_entity = std::make_shared<bpp::bpp_string>();
	case_pattern_header_entity->set_containing_class(case_pattern_entity->get_containing_class());
	case_pattern_header_entity->inherit(case_pattern_entity);

	entity_stack.push(case_pattern_header_entity);
}

void BashppListener::exitBashCasePatternHeader(std::shared_ptr<AST::BashCasePatternHeader> node) {
	std::shared_ptr<bpp::bpp_string> case_pattern_header_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (case_pattern_header_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case pattern header entity not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::dynamic_pointer_cast<bpp::bash_case_pattern>(entity_stack.top());

	if (case_pattern_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Case pattern entity not found in the entity stack");
	}

	case_pattern_entity->get_containing_case()->add_code_to_previous_line(case_pattern_header_entity->get_pre_code());
	case_pattern_entity->get_containing_case()->add_code_to_next_line(case_pattern_header_entity->get_post_code());
	case_pattern_entity->set_pattern(case_pattern_header_entity->get_code());
}
