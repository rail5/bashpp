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
	bpp_assert(entity_stack.empty(), "Entity stack is not empty");
	auto program = std::make_unique<bpp::IR::Program>();

	program->setSourceFile(getMainSourceFile());
	if (source_index) {
		source_index->addSourceFile(getMainSourceFile());
		program->setSourceIndex(source_index.get());
		source_index->setProgram(program.get());
	}

	program->setGlobalObjectStackFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_object_stack_function));
	program->setSupershellFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_supershell_function));
	program->setRepeatFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_repeat_function));
	program->setVtableLookupFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_vtable_lookup_function));
	program->setDynamicCastFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_dynamic_cast_function));
	program->setTypeofFunction(std::make_unique<bpp::IR::Builtins::SystemFunction>(bpp::IR::Builtins::bpp_typeof_function));

	root_program = program.get();
	entity_stack.push(std::move(program));
}

template <>
void Listener::exit(Program* /*node*/) {
	bpp_assert(topmost_entity_is<bpp::IR::Program>(), "Topmost entity on stack is not a Program");
	auto program = entity_stack.pop_as<bpp::IR::Program>();

	bpp_assert(entity_stack.empty(), "Entity stack is not empty after popping Program entity");
	this->program = std::move(program);
}

} // namespace bpp::AST
