/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <cstdint>
#include <expected>

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/Object.h>
#include <IR/entities/Class.h>
#include <IR/entities/SystemFunction.h>

#include <error/SyntaxError.h>

namespace bpp::IR {

/**
 * @brief The root node of the entity tree, representing the entire Bash++ program
 */
class Program : public CodeEntity {
	private:
		OwnedEntityList<Class> classes;

		/**
		 * @brief A non-owning pointer to a class that has been preregistered with this program, but not yet added to the program.
		 * This is used to allow classes to be referenced before they are fully defined, i.e., for a class to reference itself.
		 * Once the class is fully defined, it will be added to the program and removed from this list.
		 *
		 * This is a single pointer because we are guaranteed to only ever process ONE class definition at a time
		 * (Classes cannot be nested in Bash++)
		 */
		Class* preregistered_class = nullptr;

		// System functions:
		std::unique_ptr<Builtins::SystemFunction> global_object_stack_function;
		std::unique_ptr<Builtins::SystemFunction> supershell_function;
		std::unique_ptr<Builtins::SystemFunction> repeat_function;
		std::unique_ptr<Builtins::SystemFunction> vtable_lookup_function;
		std::unique_ptr<Builtins::SystemFunction> dynamic_cast_function;
		std::unique_ptr<Builtins::SystemFunction> typeof_function;

	public:
		void addDiagnostic(bpp::ErrorHandling::Diagnostic diagnostic) {}

		/**
		 * @brief Add a class to the program
		 *
		 * @param class_entity The class to add
		 * @return std::expected<void, NameConflictError> An error if the name of the class conflicts with an existing class or object in the program
		 */
		std::expected<void, NameConflictError> addClass(std::unique_ptr<Class> class_entity);

		/**
		 * @brief Preregister a class with the program.
		 *
		 * This is used to allow classes to be referenced before they are fully defined, i.e., for a class to reference itself.
		 * Once the class is fully defined, it will be added to the program and removed from this list.
		 * @param class_entity The class to preregister
		 */
		void preregisterClass(Class* class_entity);

		/**
		 * @brief Cancel the preregistration of a class with this program.
		 *
		 * If we fail to process the class's definition, this is called to unset the 'preregistered_class' pointer,
		 * so that the class is no longer considered to be preregistered within the program.
		 */
		void unPreregisterClass() { preregistered_class = nullptr; }

		Class* getClass(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const override;
		std::vector<Class*> getAllKnownClasses() const override;
		std::size_t getNumberOfKnownClasses() const override;

		OwnedEntityList<Class> releaseOwnedClasses();

		/**
		 * @brief Take ownership of the classes of another program.
		 *
		 * This is used when including (@include) another program
		 * 
		 * @param other_program The program whose classes we are adopting
		 */
		void adoptClassesOf(IncludedProgram* other_program);

		const Program* getContainingProgram() const override { return this; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		virtual const Builtins::SystemFunction* getGlobalObjectStackFunction() const { return global_object_stack_function.get(); }
		virtual const Builtins::SystemFunction* getSupershellFunction() const { return supershell_function.get(); }
		virtual const Builtins::SystemFunction* getRepeatFunction() const { return repeat_function.get(); }
		virtual const Builtins::SystemFunction* getVtableLookupFunction() const { return vtable_lookup_function.get(); }
		virtual const Builtins::SystemFunction* getDynamicCastFunction() const { return dynamic_cast_function.get(); }
		virtual const Builtins::SystemFunction* getTypeofFunction() const { return typeof_function.get(); }

		void setGlobalObjectStackFunction(std::unique_ptr<Builtins::SystemFunction> func) { global_object_stack_function = std::move(func); }
		void setSupershellFunction(std::unique_ptr<Builtins::SystemFunction> func) { supershell_function = std::move(func); }
		void setRepeatFunction(std::unique_ptr<Builtins::SystemFunction> func) { repeat_function = std::move(func); }
		void setVtableLookupFunction(std::unique_ptr<Builtins::SystemFunction> func) { vtable_lookup_function = std::move(func); }
		void setDynamicCastFunction(std::unique_ptr<Builtins::SystemFunction> func) { dynamic_cast_function = std::move(func); }
		void setTypeofFunction(std::unique_ptr<Builtins::SystemFunction> func) { typeof_function = std::move(func); }
};

/**
 * @brief A Program node that is reached via an `@include` or `@include_always` directive, representing an included Bash++ program
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
		explicit IncludedProgram(const Program* containing_program);

		void setDynamicInclude(bool is_dynamic) { is_dynamic_include = is_dynamic; }

		// IncludedProgram override checks both its *own* classes and those of its containing program
		Class* getClass(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const override;
		std::vector<Class*> getAllKnownClasses() const override;
		std::size_t getNumberOfKnownClasses() const override;

		/// Get all classes *owned* by this IncludedProgram (i.e., not including those of its containing program)
		std::vector<Class*> getOwnedClasses() const { return Program::getAllKnownClasses(); }

		/// As with IR::Program, calling IncludedProgram::getContainingProgram() will return itself
		using Program::getContainingProgram;

		/// Explicitly request a pointer to the IncludedProgram's parent program (which may be either the root program, or an earlier IncludedProgram)
		const Program* getParentProgram() const { return Entity::getContainingProgram(); }

		const Builtins::SystemFunction* getGlobalObjectStackFunction() const override { return getParentProgram()->getGlobalObjectStackFunction(); }
		const Builtins::SystemFunction* getSupershellFunction() const override { return getParentProgram()->getSupershellFunction(); }
		const Builtins::SystemFunction* getRepeatFunction() const override { return getParentProgram()->getRepeatFunction(); }
		const Builtins::SystemFunction* getVtableLookupFunction() const override { return getParentProgram()->getVtableLookupFunction(); }
		const Builtins::SystemFunction* getDynamicCastFunction() const override { return getParentProgram()->getDynamicCastFunction(); }
		const Builtins::SystemFunction* getTypeofFunction() const override { return getParentProgram()->getTypeofFunction(); }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
};

} // namespace bpp::IR
