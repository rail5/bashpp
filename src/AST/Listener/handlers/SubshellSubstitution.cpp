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
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto subshell_substitution_entity = std::make_unique<bpp::IR::SubshellSubstitution>();
	subshell_substitution_entity->inherit(current_code_entity);
	subshell_substitution_entity->setIsCatReplacement(node->isCatReplacement());

	subshell_substitution_entity->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	entity_stack.push(std::move(subshell_substitution_entity));
}

template <>
void Listener::exit(SubshellSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::SubshellSubstitution>(), "Topmost entity is not a SubshellSubstitution when exiting SubshellSubstitution node");
	auto subshell_substitution_entity = entity_stack.pop_as<bpp::IR::SubshellSubstitution>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting SubshellSubstitution node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(subshell_substitution_entity));
}

} // namespace bpp::AST
