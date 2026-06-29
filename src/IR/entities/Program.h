/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <cstdint>

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>

namespace bpp::IR {

/**
 * @brief The root node of the entity tree, representing the entire Bash++ program
 */
class Program : public CodeEntity, public std::enable_shared_from_this<Program> {
	private:
		OwnedEntityList<Class> classes;
	public:
		void add_diagnostic(std::string, bpp::diagnostic_type, std::string, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) {}

		/**
		 * @brief Add a class to the program's list of known classes
		 *
		 * NOTE: This does *not* add the class's definition to the entity tree; it only adds it to the program's list of known classes.
		 * To place the class's definition, you must call `Program::add(entity)`, as with any other entity.
		 * 
		 * @param class_entity The class to add
		 * @return true if the class was added successfully
		 * @return false if a class with the same name already exists in the program
		 */
		bool add_class(std::shared_ptr<Class> class_entity);
		std::shared_ptr<Class> get_class(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const override { return classes.find(name, max_visible_index); }
		std::vector<std::shared_ptr<Class>> get_all_known_classes() const override { return classes.get_entities(); }
		std::size_t number_of_known_classes() const override { return classes.size(); }

		void adopt_classes_of(std::shared_ptr<IncludedProgram> other_program);

		std::weak_ptr<Program> get_containing_program() override { return weak_from_this(); }
		std::weak_ptr<const Program> get_containing_program_const() const override { return weak_from_this(); }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

/**
 * @brief A Program node that is reached via an `@include` or `@include_once` directive, representing an included Bash++ program
 * Once that `@include` directive is reached in AST traversal, the linked file is lexed, parsed, and traversed to generate an entity tree of its own.
 * The `IncludedProgram` node is the root of *that* entity tree, which becomes a subtree of the main entity tree.
 *
 * Key characteristics:
 *
 * - The `IncludedProgram` node, unlike the `Program` node, *does in fact* have a parent entity *and* a "containing program"
 *   (the program that included it; either the "main" program or an earlier include).
 *   This means, for example, that the `IncludedProgram` node can instantiate classes that were defined earlier in the main program, etc.
 *
 * - After the `IncludedProgram` node is fully traversed, its classes and objects are adopted by the main program,
 *   and become visible to the main program (and any other included programs that are traversed later).
 *
 * - If the `IncludedProgram` node is reached via a *dynamic* include, then it does not generate code.
 *   It is still semantically analyzed, but it is expected that the included program will have been compiled separately as a distinct unit,
 *   and will be linked at runtime (via `source` in codegen) instead of being inlined into the main program.
 */
class IncludedProgram : public Program {
	private:
		bool is_dynamic_include = false;
	public:
		IncludedProgram() = delete;
		explicit IncludedProgram(std::shared_ptr<Program> containing_program);

		void set_dynamic_include(bool is_dynamic) { is_dynamic_include = is_dynamic; }

		// IncludedProgram override checks both its *own* classes and those of its containing program
		std::shared_ptr<Class> get_class(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const override;
		std::vector<std::shared_ptr<Class>> get_all_known_classes() const override;

		/// Get all classes *owned* by this IncludedProgram (i.e., not including those of its containing program)
		std::vector<std::shared_ptr<Class>> get_owned_classes() const { return Program::get_all_known_classes(); }

		std::weak_ptr<Program> get_containing_program() override { return containing_program; }
		std::weak_ptr<const Program> get_containing_program_const() const override { return containing_program; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
};

} // namespace bpp::IR
