/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/BashFunction.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(BashFunction* node) {
	auto* current_code_entity = dynamic_cast<bpp::IR::CodeEntity*>(entity_stack.top());
	if (!current_code_entity) throw bpp::ErrorHandling::SyntaxError(this, node, "Function definition outside of a code entity");

	const auto& function_name = node->NAME();

	if (function_name.getValue().contains("__")) {
		show_warning(
			function_name,
			bpp::ErrorHandling::WarningType::DubiousFunctionName,
			"Function name '" + function_name.getValue() + "' contains double underscores, which are reserved for internal use. Rename to avoid collisions with built-ins or generated symbols."
		);
	}

	auto function_entity = std::make_unique<bpp::IR::BashFunction>();
	function_entity->inherit(current_code_entity);
	function_entity->setName(function_name.getValue());

	function_entity->setDefinitionPosition({
		get_current_source_file(),
		function_name.getLine(),
		function_name.getCharPositionInLine()
	});

	entity_stack.push(std::move(function_entity));
}

template <>
void Listener::exit(BashFunction* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashFunction>(), "Topmost entity on stack is not a BashFunction when exiting BashFunction node");
	auto function_entity = entity_stack.pop_as<bpp::IR::BashFunction>();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashFunction node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->add(std::move(function_entity));
}

} // namespace bpp::AST
