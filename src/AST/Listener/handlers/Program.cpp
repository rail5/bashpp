/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <IR/entities/Program.h>
#include <IR/entities/SystemFunction.h>
#include <IR/builtins.h>

#include <error/InternalError.h>

namespace bpp::AST {

template <>
void Listener::enter(Program* /*node*/) {
	if (included_type_stack.top() == IncludedType::NOT_INCLUDED) {
		// This is the root program being compiled
		bpp_assert(entity_stack.empty(), "Entity stack is not empty");
		auto program = std::make_unique<bpp::IR::Program>();

		program->setSourceFile(getMainSourceFile());
		if (source_index) source_index->addSourceFile(getMainSourceFile());

		program->setGlobalObjectStackFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_object_stack_function));
		program->setSupershellFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_supershell_function));
		program->setRepeatFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_repeat_function));
		program->setVtableLookupFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_vtable_lookup_function));
		program->setDynamicCastFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_dynamic_cast_function));
		program->setTypeofFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_typeof_function));

		root_program = program.get();
		entity_stack.push(std::move(program));
	} else {
		// This program was reached via `@include`
		bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program");
		auto* current_program = entity_stack.top_as<bpp::IR::Program>();
		auto included_program = std::make_unique<bpp::IR::IncludedProgram>(current_program);
		included_program->setDynamicInclude(included_type_stack.top() == IncludedType::DYNAMICALLY_INCLUDED);
		included_program->setSourceFile(getCurrentSourceFile());
		if (source_index) source_index->addSourceFile(getCurrentSourceFile());

		entity_stack.push(std::move(included_program));
	}
}

template <>
void Listener::exit(Program* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program");
	auto program = entity_stack.pop_as<bpp::IR::Program>();

	if (included_type_stack.top() == IncludedType::NOT_INCLUDED) {
		bpp_assert(entity_stack.empty(), "Entity stack is not empty after popping Program entity");
		this->program = std::move(program);
	} else {
		bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program");
		auto* parent_program = entity_stack.top_as<bpp::IR::Program>();
		parent_program->adoptClassesOf(static_cast<bpp::IR::IncludedProgram*>(program.get()));
		parent_program->adoptObjectsOf(program.get());
		parent_program->add(std::move(program));
	}
}

} // namespace bpp::AST
