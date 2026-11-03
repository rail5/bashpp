/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>
#include <error/InternalError.h>

#include <IR/entities/expressions/bpp/TypeofExpression.h>

namespace bpp::AST {

template <>
void Listener::enter(TypeofExpression* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	auto typeof_entity = std::make_unique<bpp::IR::TypeofExpression>();
	typeof_entity->inherit(current_code_entity);

	entity_stack.push(std::move(typeof_entity));
}

template <>
void Listener::exit(TypeofExpression* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::TypeofExpression>(), "Topmost entity is not a TypeofExpression");
	auto typeof_entity = entity_stack.pop_as<bpp::IR::TypeofExpression>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	current_code_entity->adoptObjectsOf(typeof_entity.get());
	current_code_entity->add(std::move(typeof_entity));
}

} // namespace bpp::AST
