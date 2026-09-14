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
#include <IR/entities/expressions/ObjectInstantiation.h>

#include <error/InternalError.h>
#include <error/SyntaxError.h>

namespace bpp::AST {

template <>
void Listener::enter(NewStatement* node) {
	bpp_assert(topmost_entity_is<bpp::IR::CodeEntity>(), "Topmost entity is not a CodeEntity when entering NewStatement node");
	auto current_code_entity = std::static_pointer_cast<bpp::IR::CodeEntity>(entity_stack.top());

	const auto& type = node->TYPE();
	auto class_entity = current_code_entity->getClass(type);
	if (!class_entity) {
		throw bpp::ErrorHandling::SyntaxError(this, type, "Class not found: " + type.getValue());
	}

	auto instantiation = std::make_shared<bpp::IR::ObjectInstantiation>();
	instantiation->inherit(current_code_entity);
	instantiation->setType(class_entity);
	// By not setting the "object to instantiate," we indicate that this is a heap-like instantiation (i.e., a call to @new TYPE)
	current_code_entity->add(instantiation);

	// Mark __new, __constructor as used by this instantiation
	auto new_method = class_entity->getMethod_UNSAFE("__new");
	bpp_assert(new_method != nullptr, "Class has no __new method when entering NewStatement node");
	new_method->markReferencedBy(instantiation);
	auto constructor_method = class_entity->getMethod_UNSAFE("__constructor");
	if (constructor_method) constructor_method->markReferencedBy(instantiation);

	// Mark system supershell function as used by this instantiation
	auto supershell_function = program->getSupershellFunction();
	bpp_assert(supershell_function != nullptr, "Program has no supershell function when entering NewStatement node");
	supershell_function->markReferencedBy(instantiation);
}

template <>
void Listener::exit(NewStatement* /*node*/) {}

} // namespace bpp::AST
