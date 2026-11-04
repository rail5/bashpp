/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>
#include <error/InternalError.h>

#include <IR/entities/expressions/String.h>

namespace bpp::AST {

template <>
void Listener::enter(HeredocBody* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto heredoc_body_entity = std::make_unique<bpp::IR::StringType>();
	heredoc_body_entity->inherit(current_code_entity);
	heredoc_body_entity->add("\n");
	entity_stack.push(std::move(heredoc_body_entity));
}

template <>
void Listener::exit(HeredocBody* node) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType");
	auto heredoc_body_entity = entity_stack.pop_as<bpp::IR::StringType>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->adoptObjectsOf(heredoc_body_entity.get());
	current_code_entity->add(std::move(heredoc_body_entity));
	current_code_entity->add(node->DELIMITER().getValue());
}

} // namespace bpp::AST
