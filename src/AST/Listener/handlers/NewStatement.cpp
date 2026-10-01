/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/Program.h>
#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/expressions/bpp/ObjectInstantiation.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(NewStatement* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering NewStatement node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	const auto& type = node->TYPE();
	auto* class_entity = current_code_entity->getClass(type);
	if (!class_entity) {
		throw bpp::ErrorHandling::SyntaxError(this, type, "Class not found: " + type.getValue());
	}

	auto instantiation = std::make_unique<bpp::IR::ObjectInstantiation>();
	instantiation->inherit(current_code_entity);
	instantiation->setType(class_entity);
	// By not setting the "stackLikeObject" we indicate that this is a heap-like instantiation (i.e., a call to @new TYPE)
	current_code_entity->add(std::move(instantiation));
}

template <>
void Listener::exit(NewStatement* /*node*/) {}

} // namespace bpp::AST
