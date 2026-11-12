/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <cstdint>
#include <expected>
#include <filesystem>

#include <IR/bpp.h>
#include <IR/OwnedEntityList.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/Object.h>
#include <IR/entities/Class.h>
#include <IR/entities/SystemFunction.h>

namespace bpp::IR {

class SourceIndex;

/**
 * @brief The root node of the entity tree, representing the entire Bash++ program
 */
class Program final : public CodeEntity {
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

		/// The source file for this Program entity. Used for diagnostics/LSP
		std::filesystem::path source_file;

		/// The source index for this program, used for diagnostics/LSP. Non-owning.
		SourceIndex* source_index = nullptr;
	public:
		void setSourceFile(const std::filesystem::path& source_file) { this->source_file = source_file; }
		const std::filesystem::path& getSourceFile() const { return source_file; }

		void setSourceIndex(SourceIndex* source_index) { this->source_index = source_index; }
		SourceIndex* getSourceIndex() const { return source_index; }

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

} // namespace bpp::IR
