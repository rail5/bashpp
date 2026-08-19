/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Method.h>
#include <IR/entities/expressions/ObjectReference.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

#include <span>

namespace bpp::AST {

template <>
void Listener::enter(ObjectReference* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "ObjectReference node must be inside a code entity");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	auto resolution = bpp::IR::resolve_entity(
		get_current_source_file(),
		current_code_entity,
		std::span{node->IDENTIFIERS()}
	);

	if (!resolution) {
		const auto& error = resolution.error();
		if (error.token.has_value()) {
			throw bpp::ErrorHandling::SyntaxError(this, error.token.value(), error.message);
		} else {
			throw bpp::ErrorHandling::SyntaxError(this, node, error.message);
		}
		return; // Unreachable, but keeps the compiler happy
	}

	const auto& reference_entity = resolution.value();

	reference_entity->inherit(current_code_entity);
	entity_stack.push(reference_entity);
}

template <>
void Listener::exit(ObjectReference* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::ObjectReference>(), "Topmost entity on stack is not an ObjectReference when exiting ObjectReference node");
	auto reference_entity = std::static_pointer_cast<bpp::IR::ObjectReference>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "ObjectReference node must be inside a code entity");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	current_code_entity->add(reference_entity);
}

} // namespace bpp::AST
