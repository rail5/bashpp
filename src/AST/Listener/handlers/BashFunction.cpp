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
	auto current_code_entity = std::dynamic_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	if (!current_code_entity) throw bpp::ErrorHandling::SyntaxError(this, node, "Function definition outside of a code entity");

	const auto& function_name = node->NAME();

	if (function_name.getValue().contains("__")) {
		show_warning(
			function_name,
			bpp::ErrorHandling::WarningType::DubiousFunctionName,
			"Function name '" + function_name.getValue() + "' contains double underscores, which are reserved for internal use. Rename to avoid collisions with built-ins or generated symbols."
		);
	}

	auto function_entity = std::make_shared<bpp::IR::BashFunction>();
	function_entity->inherit(current_code_entity);
	function_entity->set_name(function_name.getValue());
	entity_stack.push(function_entity);

	function_entity->set_definition_position({
		get_current_source_file(),
		function_name.getLine(),
		function_name.getCharPositionInLine()
	});
}

template <>
void Listener::exit(BashFunction* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::BashFunction>(), "Topmost entity on stack is not a BashFunction when exiting BashFunction node");
	auto function_entity = std::static_pointer_cast<bpp::IR::BashFunction>(entity_stack.top());
	entity_stack.pop();

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity on stack is not a CodeEntity when exiting BashFunction node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(function_entity);
}

} // namespace bpp::AST
