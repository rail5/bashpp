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
#include <unordered_map>

#include <IR/bpp.h>
#include <IR/OwnedEntityList.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/Object.h>
#include <IR/entities/Class.h>
#include <IR/entities/SystemFunction.h>
#include <error/Diagnostic.h>
#include <include/EntityMap.h>
#include <include/unique_deque.h>

namespace bpp::IR {

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

		/// @brief The main source file of the program, which is the file that was given to Bash++ on the command line.
		std::filesystem::path source_file;

		/**
		 * @brief A list of all include statements in the program, in the order they were encountered.
		 * This is only populated if source-indexing is enabled.
		 * This is used by the language server to resolve include statements to their target files.
		 */
		std::deque<const IncludeStatement*> include_statements;

		struct SourceFileData {
			EntityMap context_entities;
			MultipleEntityMap entity_references;
			std::vector<bpp::ErrorHandling::Diagnostic> diagnostics;
		};
		std::unordered_map<std::filesystem::path, SourceFileData> file_data;
	public:
		void setSourceFile(const std::filesystem::path& source_file) { this->source_file = source_file; file_data[source_file]; }
		const std::filesystem::path& getSourceFile() const { return source_file; }

		/**
		 * @brief Add a class to the program
		 *
		 * @param class_entity The class to add
		 * @return std::expected<const Class*, NameConflictError> The canonical address of the class that was added, or an error if the name conflicts with an existing class or object in the program
		 */
		[[nodiscard]] std::expected<const Class*, NameConflictError> addClass(std::unique_ptr<Class> class_entity);

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

		auto getAllSourceFiles() const { return file_data | std::views::keys; }

		void markContextEntity(const SourceLocationRange& range, const Entity* entity) {
			file_data[range.source_file].context_entities.insert(range.start, range.end, entity);
		}

		const Entity* getContextEntity(const SourceLocation& location) const {
			if (!file_data.contains(location.source_file)) return nullptr;
			const auto* result = file_data.at(location.source_file).context_entities.find(location.position.line, location.position.column);
			if (!result) return this; // If no context entity is found, the program itself is the context
			return result;
		}

		void addDiagnostic(const std::filesystem::path& source_file, const bpp::ErrorHandling::Diagnostic& diagnostic) {
			file_data[source_file].diagnostics.push_back(diagnostic);
		}
		std::span<const bpp::ErrorHandling::Diagnostic> getDiagnostics(const std::filesystem::path& source_file) const {
			if (!file_data.contains(source_file)) return {};
			return file_data.at(source_file).diagnostics;
		}

		void recordIncludeEntity(const IncludeStatement* include_entity) {
			include_statements.push_back(include_entity);
		}

		/**
		 * @brief Collect all definition/reference positions of all referenceable entities in the program,
		 * and mirror them into the entity_references maps owned by this program.
		 *
		 * This is done s.t. the language server can quickly look up the entity being referenced at any given position in the source files.
		 */
		void collectEntityReferences();

		const unique_deque<const Entity*>& getReferencedEntities(const SourceLocation& location) const {
			static unique_deque<const Entity*> empty_set{};
			const auto& source_file = location.source_file;
			if (!file_data.contains(source_file)) return empty_set;
			return file_data.at(source_file).entity_references.find(location.position.line, location.position.column);
		}

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
