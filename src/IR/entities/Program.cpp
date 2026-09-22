/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Program.h"
#include "Class.h"

#include <error/InternalError.h>

namespace bpp::IR {

bool Program::addClass(std::shared_ptr<Class> class_entity) {
	if (classes.find(class_entity->getName())) return false; // Class with this name already exists

	return classes.add(class_entity);
}

void Program::adoptClassesOf(std::shared_ptr<IncludedProgram> other_program) {
	bpp_assert(other_program != nullptr, "Other program pointer is null");
	for (const auto& class_entity : other_program->getOwnedClasses()) {
		if (!this->addClass(class_entity)) {
			throw bpp::ErrorHandling::InternalError("adopt_classes_of() failed to adopt class '" + class_entity->getName() + "' from another program");
		}
	}
}

bpp::CodeGen::CodeSegment Program::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "State pointer is null");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("#!/usr/bin/env bash\n");

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
	code.absorb_all_to_pre(this->dynamic_cast_function->generateCode(state));
	code.absorb_all_to_pre(this->typeof_function->generateCode(state));
	code.absorb_all_to_pre(this->repeat_function->generateCode(state, state->requires_repeat_function));

	code.add_post_code("\nbpp____destroy_objectStack\n");
	// Destroy the global object stack at the end of the program, so that all destructors are called for any stack-like objects that were instantiated

	return code;
}


IncludedProgram::IncludedProgram(std::shared_ptr<Program> containing_program) {
	bpp_assert(containing_program != nullptr, "Containing program pointer is null");
	// Inherit the containing program, so that this included program can see all of its classes
	this->inherit(containing_program);
	this->setContainingProgram(containing_program);

	this->setGlobalObjectStackFunction(containing_program->getGlobalObjectStackFunction());
	this->setSupershellFunction(containing_program->getSupershellFunction());
	this->setRepeatFunction(containing_program->getRepeatFunction());
	this->setVtableLookupFunction(containing_program->getVtableLookupFunction());
	this->setDynamicCastFunction(containing_program->getDynamicCastFunction());
	this->setTypeofFunction(containing_program->getTypeofFunction());
}

std::shared_ptr<Class> IncludedProgram::getClass(const std::string& name, std::size_t max_visible_index) const {
	// First, check if this included program has a class with this name
	auto owned_class = Program::getClass(name, max_visible_index);
	if (owned_class) return owned_class;

	// If not, check the containing program (the program that included this one)
	bpp_assert(!getParentProgram().expired(), "IncludedProgram does not have a containing program");
	return getParentProgram().lock()->getClass(name, max_visible_index);
}

std::vector<std::shared_ptr<Class>> IncludedProgram::getAllKnownClasses() const {
	// Get all classes from this included program
	auto owned_classes = Program::getAllKnownClasses();

	// Get all classes from the containing program (the program that included this one)
	bpp_assert(!getParentProgram().expired(), "IncludedProgram does not have a containing program");
	const auto& containing_program_classes = getParentProgram().lock()->getAllKnownClasses();

	// Combine the two lists of classes
	owned_classes.insert(owned_classes.end(), containing_program_classes.begin(), containing_program_classes.end());

	return owned_classes;
}

std::size_t IncludedProgram::getNumberOfKnownClasses() const {
	bpp_assert(!getParentProgram().expired(), "IncludedProgram does not have a containing program");
	std::size_t owned_count = Program::getNumberOfKnownClasses();
	return owned_count + getParentProgram().lock()->getNumberOfKnownClasses();
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
