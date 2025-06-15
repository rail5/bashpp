/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

/**
* This file contains parser rules for all of:
* 		1. Bash_if_statement
* 		2. Bash_if_root_branch
* 		3. Bash_if_else_branch
* 		4. Bash_if_condition
*/

#include "../BashppListener.h"

void BashppListener::enterBash_if_statement(BashppParser::Bash_if_statementContext *ctx) {
	skip_syntax_errors
	/**
	 * Bash if statement take the form
	 * 	if CONDITION; then
	 * 		...
	 * 	elif CONDITION; then
	 * 		...
	 * 	else
	 * 		...
	 * 	fi
	 * 
	 * The elif/else branches will be caught by the Bash_if_root_branch and Bash_if_else_branch contexts
	 * 	Both of those will be children of the Bash_if_statement context in the parse tree
	 * 
	 * The only thing that we need to be careful about is this:
	 * 		All of the pre-code which is generated INSIDE of the if CONDITION
	 * 		Should be placed before the if statement altogether, not only before the condition
	 * 		Likewise all of the post-code for the condition should be placed after the if statement
	 * 
	 * For example, consider the following code:
	 * 		if [[ -f "@this.filePath" ]]; then
	 * 			...
	 * 		elif [[ -f "@this.otherFilePath" ]]; then
	 * 			...
	 * 		fi
	 * 
	 * Pre-code has to be generated in order to access @this.filePath and @this.otherFilePath
	 * The pre-code converts these references into ordinary variable references which we can substitute directly into the if condition
	 * 		As in, for example, swapping 'if [[ -f "@this.filePath" ]]' with 'if [[ -f "$filePath" ]]'
	 * 		Although we have better naming conventions for the variables, this is just an example
	 * 
	 * If we weren't careful, and just placed the pre-code the way we always do, we would end up with something like this:
	 * 		$filePath={whatever we need to do to access @this.filePath}
	 * 		if [[ -f "$filePath" ]]; then
	 * 			...
	 * 			$otherFilePath={whatever we need to do to access @this.otherFilePath}
	 * 		elif [[ -f "$otherFilePath" ]]; then
	 * 			...
	 * 		fi
	 * 
	 * As you can tell from the above, the $otherFilePath assignment is INSIDE the first branch of the if statement
	 * 		And therefore is completely useless -- if the first branch is taken, the second branch will never be taken
	 * 		And in the event that the second branch IS taken, $otherFilePath will not be defined
	 * 
	 * So, what we have to do instead is a bit more like:
	 * 		$filePath={whatever we need to do to access @this.filePath}
	 * 		$otherFilePath={whatever we need to do to access @this.otherFilePath}
	 * 		if [[ -f "$filePath" ]]; then
	 * 			...
	 * 		elif [[ -f "$otherFilePath" ]]; then
	 * 			...
	 * 		fi
	 */

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw_syntax_error(ctx->bash_if_root_branch()->BASH_KEYWORD_IF(), "If statement outside of code entity");
	}

	// Create a new code entity for the if statement
	std::shared_ptr<bpp::bash_if> if_statement_entity = std::make_shared<bpp::bash_if>();

	// Inherit the current code entity
	if_statement_entity->set_containing_class(current_code_entity->get_containing_class());
	if_statement_entity->inherit(current_code_entity);

	// Push the entity onto the stack
	entity_stack.push(if_statement_entity);
}

void BashppListener::exitBash_if_statement(BashppParser::Bash_if_statementContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_if> if_statement_entity = std::dynamic_pointer_cast<bpp::bash_if>(entity_stack.top());

	if (if_statement_entity == nullptr) {
		throw internal_error("If statement context was not found in the entity stack", ctx);
	}

	entity_stack.pop();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		throw internal_error("Code entity not found in the entity stack", ctx);
	}

	current_code_entity->add_code_to_previous_line(if_statement_entity->get_conditional_branch_pre_code());

	for (const auto& branch : if_statement_entity->get_conditional_branches()) {
		current_code_entity->add_code(branch.first);
		current_code_entity->add_code(branch.second);
	}

	current_code_entity->add_code("fi\n");
	current_code_entity->add_code(if_statement_entity->get_conditional_branch_post_code());
}

void BashppListener::enterBash_if_root_branch(BashppParser::Bash_if_root_branchContext *ctx) {
	skip_syntax_errors
	// Get the if statement entity
	std::shared_ptr<bpp::bash_if> if_statement_entity = std::dynamic_pointer_cast<bpp::bash_if>(entity_stack.top());
	if (if_statement_entity == nullptr) {
		throw internal_error("If statement entity not found in the entity stack", ctx);
	}

	if_statement_entity->new_branch();
	if_statement_entity->add_condition_code("if ");

	std::shared_ptr<bpp::bash_if_branch> condition_entity = std::make_shared<bpp::bash_if_branch>();
	condition_entity->set_containing_class(if_statement_entity->get_containing_class());
	condition_entity->inherit(if_statement_entity);
	condition_entity->set_if_statement(if_statement_entity);

	entity_stack.push(condition_entity);
}

void BashppListener::exitBash_if_root_branch(BashppParser::Bash_if_root_branchContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_if_branch> condition_entity = std::dynamic_pointer_cast<bpp::bash_if_branch>(entity_stack.top());
	if (condition_entity == nullptr) {
		throw internal_error("Condition entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_if> if_statement_entity = condition_entity->get_if_statement();
	if (if_statement_entity == nullptr) {
		throw internal_error("If statement entity not found", ctx);
	}

	if_statement_entity->add_branch_code(condition_entity->get_pre_code());
	if_statement_entity->add_branch_code(condition_entity->get_code());
	if_statement_entity->add_branch_code(condition_entity->get_post_code());
}

void BashppListener::enterBash_if_else_branch(BashppParser::Bash_if_else_branchContext *ctx) {
	skip_syntax_errors
	// Get the if statement entity
	std::shared_ptr<bpp::bash_if> if_statement_entity = std::dynamic_pointer_cast<bpp::bash_if>(entity_stack.top());
	if (if_statement_entity == nullptr) {
		throw internal_error("If statement entity not found in the entity stack", ctx);
	}

	if_statement_entity->new_branch();

	if (ctx->BASH_KEYWORD_ELIF() != nullptr) {
		if_statement_entity->add_condition_code("elif ");
	} else {
		if_statement_entity->add_condition_code("else\n");
	}

	std::shared_ptr<bpp::bash_if_branch> condition_entity = std::make_shared<bpp::bash_if_branch>();
	condition_entity->set_containing_class(if_statement_entity->get_containing_class());
	condition_entity->inherit(if_statement_entity);
	condition_entity->set_if_statement(if_statement_entity);

	entity_stack.push(condition_entity);
}

void BashppListener::exitBash_if_else_branch(BashppParser::Bash_if_else_branchContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bash_if_branch> condition_entity = std::dynamic_pointer_cast<bpp::bash_if_branch>(entity_stack.top());
	if (condition_entity == nullptr) {
		throw internal_error("Condition entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bash_if> if_statement_entity = condition_entity->get_if_statement();
	if (if_statement_entity == nullptr) {
		throw internal_error("If statement entity not found", ctx);
	}

	if_statement_entity->add_branch_code(condition_entity->get_pre_code());
	if_statement_entity->add_branch_code(condition_entity->get_code());
	if_statement_entity->add_branch_code(condition_entity->get_post_code());
}

void BashppListener::enterBash_if_condition(BashppParser::Bash_if_conditionContext *ctx) {
	skip_syntax_errors
	// Get the if branch entity
	std::shared_ptr<bpp::bash_if_branch> if_branch_entity = std::dynamic_pointer_cast<bpp::bash_if_branch>(entity_stack.top());
	if (if_branch_entity == nullptr) {
		throw internal_error("'If' branch entity not found in the entity stack", ctx);
	}

	// Create a new code entity for the condition
	std::shared_ptr<bpp::bpp_string> condition_entity = std::make_shared<bpp::bpp_string>();
	condition_entity->set_containing_class(if_branch_entity->get_containing_class());
	condition_entity->inherit(if_branch_entity);

	// Push the entity onto the stack
	entity_stack.push(condition_entity);
}

void BashppListener::exitBash_if_condition(BashppParser::Bash_if_conditionContext *ctx) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_string> condition_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	if (condition_entity == nullptr) {
		throw internal_error("Condition entity not found in the entity stack", ctx);
	}

	entity_stack.pop();

	// Get the if branch entity
	std::shared_ptr<bpp::bash_if_branch> if_branch_entity = std::dynamic_pointer_cast<bpp::bash_if_branch>(entity_stack.top());
	if (if_branch_entity == nullptr) {
		throw internal_error("'If' branch entity not found in the entity stack", ctx);
	}

	// Get the if statement entity
	std::shared_ptr<bpp::bash_if> if_statement_entity = if_branch_entity->get_if_statement();
	if (if_statement_entity == nullptr) {
		throw internal_error("If statement entity not found", ctx);
	}

	if_statement_entity->add_conditional_branch_pre_code(condition_entity->get_pre_code());
	if_statement_entity->add_conditional_branch_post_code(condition_entity->get_post_code());
	if_statement_entity->add_condition_code(condition_entity->get_code() + "; then\n");
}
