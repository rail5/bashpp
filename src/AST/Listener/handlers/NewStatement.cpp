/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/CodeEntity.h>
#include <IR/entities/Class.h>
#include <IR/entities/Program.h>
#include <IR/entities/Method.h>
#include <IR/entities/expressions/Supershell.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(NewStatement* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering NewStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	const auto& type = node->TYPE();
	auto class_entity = current_code_entity->get_class(type);
	if (!class_entity) {
		throw bpp::ErrorHandling::SyntaxError(this, type, "Class not found: " + type.getValue());
	}

	// Call __new in a supershell
	auto supershell = std::make_shared<bpp::IR::Supershell>();
	supershell->inherit(current_code_entity);
	supershell->add(class_entity->get_method_UNSAFE("__new")->get_address());

	program->get_supershell_function()->mark_referenced_by(supershell);

	current_code_entity->add(supershell);
}

template <>
void Listener::exit(NewStatement* /*node*/) {}

} // namespace bpp::AST
