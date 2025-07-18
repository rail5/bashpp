/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

/**
* This file contains parser rules for all of:
* 		1. Bash_case_statement
* 		2. Bash_case_pattern
* 		3. Bash_case_pattern_header
*/

#include "../BashppListener.h"

void BashppListener::enterBash_case_statement(BashppParser::Bash_case_statementContext *ctx) {
	skip_syntax_errors
	/**
	 * Bash case statements take the form
	 * 		case (something) in
	 * 			pattern1)
	 * 				...
	 * 				;;
	 * 			...
	 * 		esac
	 * 
	 * The code to handle each pattern will be caught by the Bash_case_pattern context
	 * 	These will be children of the Bash_case_statement context in the parse tree
	 * 
	 * The patterns to be matched will be caught by the Bash_case_pattern_header context
	 * 	These will be children of the Bash_case_pattern context in the parse tree
	 */

	 std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	 if (current_code_entity == nullptr) {
		 throw_syntax_error(ctx->BASH_KEYWORD_CASE(), "Case statement outside of code entity");
	 }

	 std::shared_ptr<bpp::bash_case> case_statement_entity = std::make_shared<bpp::bash_case>();
	 case_statement_entity->set_containing_class(current_code_entity->get_containing_class());
	 case_statement_entity->inherit(current_code_entity);

	 entity_stack.push(case_statement_entity);
}

void BashppListener::exitBash_case_statement(BashppParser::Bash_case_statementContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_case> case_statement_entity = std::dynamic_pointer_cast<bpp::bash_case>(entity_stack.top());

	if (case_statement_entity == nullptr) {
		throw internal_error("Case statement entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw internal_error("Current code entity not found in the entity stack", ctx);
	}

	current_code_entity->add_code_to_previous_line(case_statement_entity->get_pre_code());
	current_code_entity->add_code_to_next_line(case_statement_entity->get_post_code());
	current_code_entity->add_code("case " + case_statement_entity->get_code() + " in\n" + case_statement_entity->get_cases() + "\nesac\n");
}

void BashppListener::enterBash_case_pattern(BashppParser::Bash_case_patternContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_case> case_statement_entity = std::dynamic_pointer_cast<bpp::bash_case>(entity_stack.top());

	if (case_statement_entity == nullptr) {
		throw internal_error("Case statement entity not found in the entity stack", ctx);
	}

	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::make_shared<bpp::bash_case_pattern>();
	case_pattern_entity->set_containing_class(case_statement_entity->get_containing_class());
	case_pattern_entity->inherit(case_statement_entity);

	case_pattern_entity->set_containing_case(case_statement_entity);

	entity_stack.push(case_pattern_entity);

	case_pattern_entity->set_definition_position(
		source_file,
		ctx->start->getLine(),
		ctx->start->getCharPositionInLine() + 1
	);
}

void BashppListener::exitBash_case_pattern(BashppParser::Bash_case_patternContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::dynamic_pointer_cast<bpp::bash_case_pattern>(entity_stack.top());

	if (case_pattern_entity == nullptr) {
		throw internal_error("Case pattern entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_case> case_statement_entity = std::dynamic_pointer_cast<bpp::bash_case>(entity_stack.top());

	if (case_statement_entity == nullptr) {
		throw internal_error("Case statement entity not found in the entity stack", ctx);
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
		ctx->BASH_CASE_PATTERN_DELIM()->getSymbol()->getLine(),
		ctx->BASH_CASE_PATTERN_DELIM()->getSymbol()->getCharPositionInLine() + 1,
		case_pattern_entity
	);
}

void BashppListener::enterBash_case_pattern_header(BashppParser::Bash_case_pattern_headerContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::dynamic_pointer_cast<bpp::bash_case_pattern>(entity_stack.top());

	if (case_pattern_entity == nullptr) {
		throw internal_error("Case pattern entity not found in the entity stack", ctx);
	}

	std::shared_ptr<bpp::bpp_string> case_pattern_header_entity = std::make_shared<bpp::bpp_string>();
	case_pattern_header_entity->set_containing_class(case_pattern_entity->get_containing_class());
	case_pattern_header_entity->inherit(case_pattern_entity);

	entity_stack.push(case_pattern_header_entity);
}

void BashppListener::exitBash_case_pattern_header(BashppParser::Bash_case_pattern_headerContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> case_pattern_header_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (case_pattern_header_entity == nullptr) {
		throw internal_error("Case pattern header entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_case_pattern> case_pattern_entity = std::dynamic_pointer_cast<bpp::bash_case_pattern>(entity_stack.top());

	if (case_pattern_entity == nullptr) {
		throw internal_error("Case pattern entity not found in the entity stack", ctx);
	}

	case_pattern_entity->get_containing_case()->add_code_to_previous_line(case_pattern_header_entity->get_pre_code());
	case_pattern_entity->get_containing_case()->add_code_to_next_line(case_pattern_header_entity->get_post_code());
	case_pattern_entity->set_pattern(case_pattern_header_entity->get_code());
}
