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
	bpp_assert(other_program != nullptr, "adopt_classes_of() was given a null other_program pointer");
	for (const auto& class_entity : other_program->getOwnedClasses()) {
		if (!this->addClass(class_entity)) {
			throw bpp::ErrorHandling::InternalError("adopt_classes_of() failed to adopt class '" + class_entity->getName() + "' from another program");
		}
	}
}

bpp::CodeGen::CodeSegment Program::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Program::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("#!/usr/bin/env bash\n");

	if (state->target_bash_version < BashVersion{5, 3}) {
		code.absorb_all_to_pre(this->supershell_function->generateCode(state));
	} // Bash>=5.3 has a native supershell implementation, skip adding our own

	code.absorb_all_to_pre(this->repeat_function->generateCode(state));
	code.absorb_all_to_pre(this->vtable_lookup_function->generateCode(state));
	code.absorb_all_to_pre(this->dynamic_cast_function->generateCode(state));
	code.absorb_all_to_pre(this->typeof_function->generateCode(state));

	code.egalitarian_merge(CodeEntity::generateCode(state));

	return code;
}


IncludedProgram::IncludedProgram(std::shared_ptr<Program> containing_program) {
	bpp_assert(containing_program != nullptr, "IncludedProgram constructor was given a null containing_program pointer");
	// Inherit the containing program, so that this included program can see all of its classes
	this->inherit(containing_program);
	this->setContainingProgram(containing_program);

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
	bpp_assert(!getContainingProgram().expired(), "IncludedProgram does not have a containing program");
	return getContainingProgram().lock()->getClass(name, max_visible_index);
}

std::vector<std::shared_ptr<Class>> IncludedProgram::getAllKnownClasses() const {
	// Get all classes from this included program
	auto owned_classes = Program::getAllKnownClasses();

	// Get all classes from the containing program (the program that included this one)
	bpp_assert(!getContainingProgram().expired(), "IncludedProgram does not have a containing program");
	const auto& containing_program_classes = getContainingProgram().lock()->getAllKnownClasses();

	// Combine the two lists of classes
	owned_classes.insert(owned_classes.end(), containing_program_classes.begin(), containing_program_classes.end());

	return owned_classes;
}

bpp::CodeGen::CodeSegment IncludedProgram::generateCode(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "IncludedProgram::generate_code() should be called with a non-null state pointer");
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
