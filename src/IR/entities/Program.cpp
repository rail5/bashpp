/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Program.h"
#include "Class.h"

#include <error/InternalError.h>

namespace bpp::IR {

std::expected<void, NameConflictError> Program::addClass(std::unique_ptr<Class> class_entity) {
	// If this class was pre-registered, remove it from the preregistered list, since it is now being fully defined
	preregistered_class = nullptr;

	if (classes.find(class_entity->viewName())) return std::unexpected(NameConflictError::EXISTING_CLASS);
	if (getObject(class_entity->getName())) return std::unexpected(NameConflictError::EXISTING_OBJECT);

	classes.add(std::move(class_entity));
	return {};
}

void Program::preregisterClass(Class* class_entity) {
	bpp_assert(class_entity != nullptr, "Class pointer is null");
	preregistered_class = class_entity;
}

Class* Program::getClass(const std::string& name, std::size_t max_visible_index) const {
	Class* res = classes.find(name, max_visible_index);
	if (res) return res;

	// If not found in the owned classes, check the preregistered classes
	if (preregistered_class && preregistered_class->getName() == name) {
		if (max_visible_index <= classes.size()) return nullptr; // The preregistered class is not visible at this index
		return preregistered_class;
	}
	return nullptr;
}

std::vector<Class*> Program::getAllKnownClasses() const {
	std::vector<Class*> result;
	result.reserve(classes.size() + 1);
	std::transform(classes.view_entities().begin(), classes.view_entities().end(), std::back_inserter(result), [](const std::unique_ptr<Class>& class_ptr) { return class_ptr.get(); });
	if (preregistered_class) result.push_back(preregistered_class);
	return result;
}

std::size_t Program::getNumberOfKnownClasses() const {
	return classes.size() + (preregistered_class ? 1 : 0);
}

void Program::adoptClassesOf(IncludedProgram* other_program) {
	bpp_assert(other_program != nullptr, "Other program pointer is null");
	for (auto&& class_entity : other_program->releaseOwnedClasses().release_entities()) {
		std::string name = class_entity->getName();
		if (!this->addClass(std::move(class_entity))) {
			throw bpp::ErrorHandling::InternalError("adopt_classes_of() failed to adopt class '" + name + "' from another program");
		}
	}
}

bpp::IR::OwnedEntityList<Class> Program::releaseOwnedClasses() {
	return std::move(classes);
}

bpp::CodeGen::CodeSegment Program::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("#!/usr/bin/env bash\n");

	for (const auto& class_entity : classes.view_entities()) {
		code.absorb_all_to_main(class_entity->generateCode(state));
	}

	code.absorb_all_to_main(CodeEntity::generateCode(state));

	if (!state->requires_global_object_stack) {
		code.add_pre_code("if ! type bpp____destroy_objectStack &>/dev/null; then\nbpp____destroy_objectStack() { return 0; }\nfi\n");
		// Why?
		// Suppose two files, A and B, are compiled separately, and 'A' dynamically includes 'B'.
		// File A instantiates a stack-like object, which registers itself in the global object stack.
		// File B does NOT instantiate any stack-like objects, and therefore does not require the global object stack.
		// File B nevertheless contains an early-exit path (inside a function, or a method) that calls 'exit'
		//
		// Because we can't know (at the time of compiling file B) whether the "full program" (file A + file B) will have a global object stack,
		// we have to just call the destroy_objectStack function anyway, even though file B itself doesn't require it.
		//
		// So, just in case the full program doesn't require a global object stack, we add a no-op destroy_objectStack function here,
		// so that we don't get any "command not found" errors.
	}

	code.absorb_all_to_pre(this->global_object_stack_function->generateCode(state, state->requires_global_object_stack));

	code.add_pre_code("__scopeFrames=(0)\n");

	if (state->target_bash_version < BashVersion{5, 3}) {
		code.absorb_all_to_pre(this->supershell_function->generateCode(state, state->requires_supershell_function));
	} // Bash>=5.3 has a native supershell implementation, skip adding our own

	code.absorb_all_to_pre(this->vtable_lookup_function->generateCode(state, state->requires_vtable_lookup_function));
	code.absorb_all_to_pre(this->dynamic_cast_function->generateCode(state, state->requires_dynamic_cast_function));
	code.absorb_all_to_pre(this->typeof_function->generateCode(state, state->requires_typeof_function));
	code.absorb_all_to_pre(this->repeat_function->generateCode(state, state->requires_repeat_function));

	code.add_post_code("\nbpp____destroy_objectStack\n");
	// Destroy the global object stack at the end of the program, so that all destructors are called for any stack-like objects that were instantiated

	return code;
}


IncludedProgram::IncludedProgram(const Program* containing_program) {
	bpp_assert(containing_program != nullptr, "Containing program pointer is null");
	// Inherit the containing program, so that this included program can see all of its classes
	this->inherit(containing_program);
	this->setContainingProgram(containing_program);
}

Class* IncludedProgram::getClass(const std::string& name, std::size_t max_visible_index) const {
	// First, check if this included program has a class with this name
	auto* owned_class = Program::getClass(name, max_visible_index);
	if (owned_class) return owned_class;

	// If not, check the containing program (the program that included this one)
	bpp_assert(getParentProgram(), "IncludedProgram does not have a containing program");
	return getParentProgram()->getClass(name, max_visible_index);
}

std::vector<Class*> IncludedProgram::getAllKnownClasses() const {
	// Get all classes from this included program
	auto owned_classes = Program::getAllKnownClasses();

	// Get all classes from the containing program (the program that included this one)
	bpp_assert(getParentProgram(), "IncludedProgram does not have a containing program");
	const auto& containing_program_classes = getParentProgram()->getAllKnownClasses();

	// Combine the two lists of classes
	owned_classes.insert(owned_classes.end(), containing_program_classes.begin(), containing_program_classes.end());

	return owned_classes;
}

std::size_t IncludedProgram::getNumberOfKnownClasses() const {
	bpp_assert(getParentProgram(), "IncludedProgram does not have a containing program");
	std::size_t owned_count = Program::getNumberOfKnownClasses();
	return owned_count + getParentProgram()->getNumberOfKnownClasses();
}

bpp::CodeGen::CodeSegment IncludedProgram::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment code;

	// If this included program is a dynamic include, it does not generate code
	if (is_dynamic_include) return code;

	// Otherwise, generate code for this included program as normal
	// Note that we deliberately call CodeEntity::generate_code() here, rather than Program::generate_code(),
	// because Program::generate_code() would duplicate the shebang and the system functions
	// NOLINTNEXTLINE(bugprone-parent-virtual-call)
	code.egalitarian_merge(CodeEntity::generateCode(state));

	return code;
}

} // namespace bpp::IR
