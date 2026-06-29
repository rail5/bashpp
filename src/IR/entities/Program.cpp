/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Program.h"
#include "Class.h"

#include <error/InternalError.h>

namespace bpp::IR {

bool Program::add_class(std::shared_ptr<Class> class_entity) {
	if (classes.find(class_entity->get_name())) return false; // Class with this name already exists

	return classes.add(class_entity);
}

void Program::adopt_classes_of(std::shared_ptr<IncludedProgram> other_program) {
	bpp_assert(other_program != nullptr, "adopt_classes_of() was given a null other_program pointer");
	for (const auto& class_entity : other_program->get_owned_classes()) {
		if (!this->add_class(class_entity)) {
			throw bpp::ErrorHandling::InternalError("adopt_classes_of() failed to adopt class '" + class_entity->get_name() + "' from another program");
		}
	}
}

bpp::CodeGen::CodeSegment Program::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "Program::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	code.add_pre_code("#!/usr/bin/env bash\n");
	code.egalitarian_merge(CodeEntity::generate_code(state));

	return code;
}


IncludedProgram::IncludedProgram(std::shared_ptr<Program> containing_program) {
	bpp_assert(containing_program != nullptr, "IncludedProgram constructor was given a null containing_program pointer");
	// Inherit the containing program, so that this included program can see all of its classes
	this->inherit(containing_program);
	this->containing_program = containing_program;
	this->parent_entity = containing_program;
}

std::shared_ptr<Class> IncludedProgram::get_class(const std::string& name, std::size_t max_visible_index) const {
	// First, check if this included program has a class with this name
	auto owned_class = Program::get_class(name, max_visible_index);
	if (owned_class) return owned_class;

	// If not, check the containing program (the program that included this one)
	bpp_assert(!containing_program.expired(), "IncludedProgram does not have a containing program");
	return containing_program.lock()->get_class(name, max_visible_index);
}

std::vector<std::shared_ptr<Class>> IncludedProgram::get_all_known_classes() const {
	// Get all classes from this included program
	auto owned_classes = Program::get_all_known_classes();

	// Get all classes from the containing program (the program that included this one)
	bpp_assert(!containing_program.expired(), "IncludedProgram does not have a containing program");
	const auto& containing_program_classes = containing_program.lock()->get_all_known_classes();

	// Combine the two lists of classes
	owned_classes.insert(owned_classes.end(), containing_program_classes.begin(), containing_program_classes.end());

	return owned_classes;
}

bpp::CodeGen::CodeSegment IncludedProgram::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp_assert(state != nullptr, "IncludedProgram::generate_code() should be called with a non-null state pointer");
	bpp::CodeGen::CodeSegment code;

	// If this included program is a dynamic include, it does not generate code
	if (is_dynamic_include) return code;

	// Otherwise, generate code for this included program as normal
	code.egalitarian_merge(CodeEntity::generate_code(state));

	return code;
}

} // namespace bpp::IR
