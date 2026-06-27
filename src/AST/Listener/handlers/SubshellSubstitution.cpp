/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/SubshellSubstitution.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(SubshellSubstitution* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering SubshellSubstitution node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto subshell_substitution_entity = std::make_shared<bpp::IR::SubshellSubstitution>();
	subshell_substitution_entity->set_containing_class(current_code_entity->get_containing_class().lock());
	subshell_substitution_entity->inherit(current_code_entity);
	subshell_substitution_entity->setIsCatReplacement(node->isCatReplacement());
	entity_stack.push(subshell_substitution_entity);

	subshell_substitution_entity->set_definition_position({
		source_file,
		node->getLine(),
		node->getCharPositionInLine()
	});
}

template <>
void Listener::exit(SubshellSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::SubshellSubstitution>(), "Topmost entity is not a SubshellSubstitution when exiting SubshellSubstitution node");
	auto subshell_substitution_entity = std::static_pointer_cast<bpp::IR::SubshellSubstitution>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting SubshellSubstitution node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(subshell_substitution_entity);
}

} // namespace bpp::AST
