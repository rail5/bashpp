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

#include <deque>

namespace bpp::AST {

template <>
void Listener::enter(ObjectReference* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "ObjectReference node must be inside a code entity");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	std::deque<AST::Token<std::string>> identifiers;
	identifiers.push_back(node->IDENTIFIER());
	std::copy(node->IDENTIFIERS().begin(), node->IDENTIFIERS().end(), std::back_inserter(identifiers));

	auto resolution = bpp::IR::resolve_entity(
		get_current_source_file(),
		current_code_entity,
		identifiers
	);

	if (resolution.error_message.has_value()) {
		if (resolution.error_token.has_value()) {
			throw bpp::ErrorHandling::SyntaxError(this, resolution.error_token.value(), resolution.error_message.value());
		} else {
			throw bpp::ErrorHandling::SyntaxError(this, node, resolution.error_message.value());
		}
	}

	current_code_entity->add(resolution.ref);
}

template <>
void Listener::exit(ObjectReference* node) {
}

} // namespace bpp::AST
