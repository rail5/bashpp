/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) {
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Subshell substitution outside of code entity");
	}

	// Create a new code entity for the subshell
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::make_shared<bpp::bpp_string>();
	subshell_entity->set_containing_class(code_entity->get_containing_class());
	subshell_entity->inherit(code_entity);

	if (node->isCatReplacement()) {
		// "Perfect forwarding" is required for "cat replacement" subshells (i.e., ones of the form $(< file))
		// Reason being: those "cat replacements" cannot contain any non-trivial logic such as function calls etc,
		// and can *only* contain the redirection
		// Meaning that this entity needs to assert to all other entities: "I need total control of where my pre-code and post-code go."
		// (The definition of "perfect forwarding," that is, keeping the 3 code buffers separate)
		subshell_entity->set_requires_perfect_forwarding(true);
	}

	// Push the subshell entity onto the entity stack
	entity_stack.push(subshell_entity);

	subshell_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);
}

void BashppListener::exitSubshellSubstitution(std::shared_ptr<AST::SubshellSubstitution> node) {
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (subshell_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Subshell substitution context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	

	if (node->isCatReplacement()) {
		// If this is a `$(< file)` "cat replacement" subshell
		// (i.e., a shell-builtin replacement for the `cat` utility to read from some file),
		// then it cannot contain any nontrivial logic such as function calls etc, but can *only* contain the redirection
		// therefore we need to make sure that the entity's pre-code comes *before* the entity,
		// and its post-code comes *after* the entity
		// e.g.: If, as part of this "cat replacement," we fetch an object's data member,
		// then the temporary variables for the fetch need to come *before* the $(< ...) business,
		//  (inside the $(< ...) should *only* be the final variable reference)
		// and the clean-up should come *after* the $(< ...)
		current_code_entity->add_code_to_previous_line(subshell_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(subshell_entity->get_post_code());
		current_code_entity->add_code("$(" + subshell_entity->get_code() + ")");

		// Only *now* should we destruct local objects and flush the code buffers
		// -- since these operations can potentially rotate the buffers (e.g., moving the contents of the "code" buffer into the "pre-code" buffer)
		// or insert new content into any of the 3 buffers
		subshell_entity->clear_all_buffers(); // To avoid sending the same code up twice
		subshell_entity->destruct_local_objects(program);
		subshell_entity->flush_code_buffers();

		current_code_entity->add_code(subshell_entity->get_pre_code());
		current_code_entity->add_code(subshell_entity->get_code());
		current_code_entity->add_code(subshell_entity->get_post_code());
	} else {
		// Otherwise, if this is a normal subshell (i.e., one containing commands),
		// Then we actually need to do the opposite: all of the pre- and post- code should be contained *within*
		// the subshell, for proper scoping concerns
		// (likewise, destructors for local objects etc should be called within the same subshell)
		subshell_entity->destruct_local_objects(program);
		subshell_entity->flush_code_buffers();
		current_code_entity->add_code("$(", false);
		current_code_entity->add_code(subshell_entity->get_pre_code());
		current_code_entity->add_code(subshell_entity->get_code());
		current_code_entity->add_code(subshell_entity->get_post_code());
		current_code_entity->add_code(")", false);
	}

	program->mark_entity(
		source_file,
		subshell_entity->get_initial_definition().line,
		subshell_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		subshell_entity
	);
}

void BashppListener::enterRawSubshell(std::shared_ptr<AST::RawSubshell> node) {
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (code_entity == nullptr) {
		throw bpp::ErrorHandling::SyntaxError(this, node, "Subshell outside of code entity");
	}

	// Create a new code entity for the subshell
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::make_shared<bpp::bpp_string>();
	subshell_entity->set_containing_class(code_entity->get_containing_class());
	subshell_entity->inherit(code_entity);

	// Push the subshell entity onto the entity stack
	entity_stack.push(subshell_entity);

	subshell_entity->set_definition_position(
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	);

	// Add the opening parenthesis to the current code entity
	// TODO(@rail5): Does the opening parenthesis properly belong as part of the subshell entity rather than its parent code entity?
	code_entity->add_code("(\n", false);
}

void BashppListener::exitRawSubshell(std::shared_ptr<AST::RawSubshell> node) {
	std::shared_ptr<bpp::bpp_string> subshell_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());

	if (subshell_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Subshell context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		throw bpp::ErrorHandling::InternalError("Containing code entity was not found in the entity stack");
	}

	subshell_entity->destruct_local_objects(program);
	subshell_entity->flush_code_buffers();

	current_code_entity->add_code(subshell_entity->get_pre_code());
	current_code_entity->add_code(subshell_entity->get_code());
	current_code_entity->add_code(subshell_entity->get_post_code());

	current_code_entity->add_code("\n)", false);

	program->mark_entity(
		source_file,
		subshell_entity->get_initial_definition().line,
		subshell_entity->get_initial_definition().column,
		node->getEndPosition().line,
		node->getEndPosition().column,
		subshell_entity
	);
}
