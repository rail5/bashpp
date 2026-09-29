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
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();

	show_warning(
		node,
		bpp::ErrorHandling::WarningType::Bash53NativeSupershell,
		"Consider using the Bash++ supershell syntax `@(...)` for wider compatibility"
	);

	auto b53_supershell_entity = std::make_unique<bpp::IR::StringType>();
	b53_supershell_entity->inherit(current_code_entity);

	b53_supershell_entity->add(node->STARTTOKEN()); // Copy the `${` or `${|` start token as RawCode into the entity

	b53_supershell_entity->setDefinitionPosition({
		get_current_source_file(),
		node->getLine(),
		node->getCharPositionInLine()
	});

	entity_stack.push(std::move(b53_supershell_entity));
}

template <>
void Listener::exit(Bash53NativeSupershell* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::StringType>(), "Topmost entity is not a StringType when exiting Bash53NativeSupershell node");
	auto b53_supershell_entity = entity_stack.pop_as<bpp::IR::StringType>();

	b53_supershell_entity->add("}"); // Copy the `}` end token as RawCode into the entity

	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when exiting Bash53NativeSupershell node");
	auto* current_code_entity = entity_stack.top_as<bpp::IR::CodeEntity>();
	current_code_entity->adoptObjectsOf(b53_supershell_entity.get());
	current_code_entity->add(std::move(b53_supershell_entity));
}

} // namespace bpp::AST
