/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/BashFunction.h>
#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashFunction* node) {
	auto* current_code_entity = dynamic_cast<bpp::IR::CodeEntity*>(entity_stack.top());
	if (!current_code_entity) error(node, "Function definition outside of a code entity");

	const auto& function_name = node->NAME();

	if (function_name.getValue().contains("__")) {
		warn(
			function_name,
			bpp::ErrorHandling::WarningType::DubiousFunctionName,
			"Function name '" + function_name.getValue() + "' contains double underscores, which are reserved for internal use. Rename to avoid collisions with built-ins or generated symbols."
		);
	}

	auto function_entity = std::make_unique<bpp::IR::BashFunction>();
	function_entity->inherit(current_code_entity);
	function_entity->setName(function_name.getValue());

	if (source_index) {
		source_index->setDefinitionPosition(
			function_entity.get(),
			{getCurrentSourceFile(),
			function_name.getLine(), function_name.getColumn(),
			function_name.getLine(), static_cast<uint32_t>(function_name.getColumn() + function_name.getValue().length())
		});

		source_index->markContextEntity(
			{getCurrentSourceFile(),
			node->getLine(), node->getColumn(),
			node->getEndPosition().line, node->getEndPosition().column},
			function_entity.get()
		);
	}

	entity_stack.push(std::move(function_entity));
}

template <>
void Listener::exit(BashFunction* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashFunction>(), "Topmost entity on stack is not a BashFunction");
	auto function_entity = entity_stack.pop_as<bpp::IR::BashFunction>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(function_entity));
}

} // namespace bpp::AST
