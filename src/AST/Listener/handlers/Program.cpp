/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(Program* /*node*/) {
	if (included_type_stack.top() == IncludedType::NOT_INCLUDED) {
		// This is the root program being compiled
		auto program = std::make_shared<bpp::IR::Program>();
		entity_stack.push(program);
		this->program = program;
	} else {
		// This program was reached via `@include`
		bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program when entering an included Program node");
		auto current_program = std::static_pointer_cast<bpp::IR::Program>(entity_stack.top());
		auto included_program = std::make_shared<bpp::IR::IncludedProgram>(current_program);
		entity_stack.push(included_program);
		included_program->set_dynamic_include(included_type_stack.top() == IncludedType::DYNAMICALLY_INCLUDED);
		current_program->add(included_program);
	}
}

template <>
void Listener::exit(Program* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program when exiting Program node");
	auto program = std::static_pointer_cast<bpp::IR::Program>(entity_stack.top());
	entity_stack.pop();

	if (included_type_stack.top() == IncludedType::NOT_INCLUDED) {
		bpp_assert(entity_stack.empty(), "Entity stack is not empty after popping Program entity");
	} else {
		bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program when exiting an included Program node");
		auto parent_program = std::static_pointer_cast<bpp::IR::Program>(entity_stack.top());
		parent_program->adopt_classes_of(std::static_pointer_cast<bpp::IR::IncludedProgram>(program));
		parent_program->adopt_objects_of(program);
	}
}

} // namespace bpp::AST
