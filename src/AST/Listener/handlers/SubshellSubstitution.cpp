/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/bash/SubshellSubstitution.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(SubshellSubstitution* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto subshell_substitution_entity = std::make_unique<bpp::IR::SubshellSubstitution>();
	subshell_substitution_entity->inherit(current_code_entity);
	subshell_substitution_entity->setIsCatReplacement(node->isCatReplacement());

	root_program->markEntity(
		getCurrentSourceFile(),
		node->getLine(), node->getCharPositionInLine(),
		node->getEndPosition().line, node->getEndPosition().column,
		subshell_substitution_entity.get()
	);

	entity_stack.push(std::move(subshell_substitution_entity));
}

template <>
void Listener::exit(SubshellSubstitution* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::SubshellSubstitution>(), "Topmost entity is not a SubshellSubstitution");
	auto subshell_substitution_entity = entity_stack.pop_as<bpp::IR::SubshellSubstitution>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(subshell_substitution_entity));
}

} // namespace bpp::AST
