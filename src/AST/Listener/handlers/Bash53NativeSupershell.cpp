/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/expressions/String.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(Bash53NativeSupershell* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering Bash53NativeSupershell node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	show_warning(
		node,
		bpp::ErrorHandling::WarningType::Bash53NativeSupershell,
		"Consider using the Bash++ supershell syntax `@(...)` for wider compatibility"
	);

	auto b53_supershell_entity = std::make_shared<bpp::IR::StringType>();
	b53_supershell_entity->set_containing_class(current_code_entity->get_containing_class().lock());
	b53_supershell_entity->inherit(current_code_entity);

	b53_supershell_entity->add(node->STARTTOKEN()); // Copy the `${` or `${|` start token as RawCode into the entity

	entity_stack.push(b53_supershell_entity);
	b53_supershell_entity->set_definition_position({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});
}

template <>
void Listener::exit(Bash53NativeSupershell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting Bash53NativeSupershell node");
	auto b53_supershell_entity = std::static_pointer_cast<bpp::IR::StringType>(entity_stack.top());
	entity_stack.pop();

	b53_supershell_entity->add("}"); // Copy the `}` end token as RawCode into the entity

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting Bash53NativeSupershell node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());
	current_code_entity->add(b53_supershell_entity);
	current_code_entity->adopt_objects_of(b53_supershell_entity);
}

} // namespace bpp::AST
