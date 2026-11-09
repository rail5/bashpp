/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>

#include <IR/bpp.h>
#include <IR/SourceIndex.h>
#include <include/BashVersion.h>
#include <error/WarningOptions.h>

/**
 * @class ProgramPool
 * @brief Manages a pool of IR::Program objects for efficient reuse and access.
 *
 * Each parsed program has its data stored in an IR::Program object.
 * The ProgramPool class allows for efficient management of these objects,
 * including adding, retrieving, and removing programs based on file paths.
 *
 * By default, it keeps a maximum of 10 programs in the pool.
 * When an 11th program is added, the oldest program is removed to make space.
 *
 * Having a program in the pool means that it has been parsed and is ready for use.
 * I.e., we do not have to re-parse the program every time we want to request information from it,
 * Such as "where was this entity defined?" or "what's the class of this object?"
 *
 * The pool is designed to be thread-safe, allowing multiple threads to access and modify the pool concurrently.
 * 
 */
class ProgramPool {
	public:
		struct ProgramAndSourceIndex {
			std::unique_ptr<bpp::IR::Program> program;
			std::unique_ptr<bpp::IR::SourceIndex> source_index;
		};
	private:
		std::size_t max_programs = 10; // Maximum number of programs to keep in the pool

		std::vector<std::shared_ptr<ProgramAndSourceIndex>> programs;
		std::unordered_map<std::filesystem::path, std::vector<std::size_t>> program_indices; // Maps file paths to program indices in the pool
		std::unordered_map<std::filesystem::path, bool> open_files; // Maps file paths to whether they are currently open
		std::unordered_map<std::filesystem::path, std::string> unsaved_changes; // Maps file paths to their unsaved contents
		BashVersion target_bash_version = {5, 2};
		bpp::ErrorHandling::WarningOptions warning_options;
		std::recursive_mutex pool_mutex; // Mutex to protect access to the pool

		// Pool snapshots:
		struct Snapshot {
			std::vector<std::shared_ptr<ProgramAndSourceIndex>> programs_snapshot; // Snapshot of the current programs in the pool
			std::unordered_map<std::filesystem::path, std::vector<std::size_t>> program_indices_snapshot; // Snapshot of the current program indices
			std::unordered_map<std::filesystem::path, bool> open_files_snapshot; // Snapshot of the current open files
			std::unordered_map<std::filesystem::path, std::string> unsaved_changes_snapshot; // Snapshot of the current unsaved changes
		};
		std::unique_ptr<Snapshot> snapshot = std::make_unique<Snapshot>();
		mutable std::recursive_mutex snapshot_mutex; // Mutex to protect access to the snapshot

		bool utf16_mode = false; // Whether to use UTF-16 mode for character counting

		void _remove_oldest_program();
		void _remove_program(std::size_t index);
		std::shared_ptr<ProgramAndSourceIndex> _parse_program(const std::filesystem::path& file_path);

		// Configurable settings
		std::vector<std::filesystem::path> include_paths;

		void update_snapshot();
		Snapshot load_snapshot() const;
	public:
		explicit ProgramPool(std::size_t max_programs = 10);

		/**
		 * @brief Add an include path for use by all future programs to be added to the pool.
		 *
		 * This mirrors the `-I` option in bpp
		 * 
		 * @param path The include path to add.
		 */
		void addIncludePath(const std::filesystem::path& path);

		/**
		 * @brief Add a warning option for use by all future programs to be added to the pool.
		 * 
		 * @param warning_option The warning option to add.
		 * @throws std::runtime_error if the warning option is invalid
		 * @see bpp::ErrorHandling::WarningOptions for valid warning options
		 */
		void addWarningOption(std::string_view warning_option);
		void setUTF16Mode(bool mode);
		bool getUTF16Mode() const;
		void setTargetBashVersion(const BashVersion& version);

		/**
		 * @brief Set the unsaved contents for a file to reflect in-editor changes.
		 *
		 * Future re-parses of the relevant program will use these contents
		 * 
		 * @param file_path The file path to set unsaved contents for.
		 * @param contents The unsaved contents of the file.
		 */
		void setUnsavedFileContents(const std::filesystem::path& file_path, const std::string& contents);
		void removeUnsavedFileContents(const std::filesystem::path& file_path);

		/**
		 * @brief Get the contents of a file, considering unsaved changes if present.
		 *
		 * If there are unsaved changes for the given file path, those contents are returned.
		 * Otherwise, the contents are read from the file on disk.
		 * 
		 * @param file_path The file path to get the contents of.
		 * @return std::string The contents of the file.
		 */
		std::string getFileContents(const std::filesystem::path& file_path);

		/**
		 * @brief Get or create a program for the given file path
		 * 
		 * If any programs in the pool report that they're associated with the given file path,
		 * the first such program is returned.
		 * If no program exists for that file, we create a new program, add it to the pool,
		 * and return the new program.
		 * If jump_queue is true, we will not create a new program if one does not already exist.
		 * Instead, we will simply return nullptr.
		 *
		 * @param file_path The source file path to get or create a program for.
		 * @param jump_queue Whether to jump the request queue. If true, the request is
		 *                   processed immediately. However, we will also refuse to create
		 *                   a new program -- if none exists, and you've asked to jump the
		 *                   queue, we will simply return nullptr.
		 * @return std::shared_ptr<ProgramAndSourceIndex> A program and source index associated with the given file path, or nullptr if no such program exists and jump_queue is true.
		 */
		std::shared_ptr<ProgramAndSourceIndex> getProgram(const std::filesystem::path& file_path, bool jump_queue = false);

		/**
		 * @brief Get all programs associated with the given file path.
		 * If no programs are associated with the given file path, we create a new
		 * program for that file path, add it to the pool, and return a vector
		 * containing just that program.
		 * 
		 * @param file_path The source file path to get or create programs for.
		 * @param jump_queue Whether to jump the request queue. If true, the request is
		 *                   processed immediately. However, we will also refuse to create
		 *                   new programs -- if none exist, and you've asked to jump the
		 *                   queue, we will simply return an empty vector.
		 * @return std::vector<std::shared_ptr<ProgramAndSourceIndex>> A vector of programs and source indices associated with the given file path. If no such programs exist and jump_queue is true, an empty vector is returned.
		 */
		std::vector<std::shared_ptr<ProgramAndSourceIndex>> getProgramsForFile(const std::filesystem::path& file_path, bool jump_queue = false);

		/**
		 * @brief Check if a program for the given file path exists in the pool.
		 * 
		 * @param file_path The source file path to check for a program.
		 * @return true if a program exists in the pool for the given file path, false otherwise.
		 */
		bool hasProgram(const std::filesystem::path& file_path);

		/**
		 * @brief Re-parse all programs associated with the given file path
		 * 
		 * @param file_path The source file which has been modified, triggering the re-parse.
		 * @return std::vector<std::shared_ptr<ProgramAndSourceIndex>> A vector of all programs and source indices that were re-parsed. If a program failed to re-parse, it will not be included in the returned vector.
		 */
		std::vector<std::shared_ptr<ProgramAndSourceIndex>> reparsePrograms(const std::string& file_path);

		/**
		 * @brief Query all programs in the pool for entities at a given source location.
		 * 
		 * @param location The source location to query for entities.
		 * @return std::vector<const bpp::IR::Entity*> A vector of all entities found at the given source location across all programs in the pool.
		 */
		std::vector<const bpp::IR::Entity*> findAllEntitiesForSymbol(const bpp::IR::SourceLocation& location) const;

		/**
		 * @brief Get the program and source index associated with a given entity.
		 * 
		 * @param entity The entity to find the associated program and source index for.
		 * @return std::shared_ptr<ProgramAndSourceIndex> A shared pointer to the program and source index associated with the given entity, or nullptr if no such program exists in the pool.
		 */
		std::shared_ptr<ProgramAndSourceIndex> getProgramForEntity(const bpp::IR::Entity* entity) const;

		/**
		 * @brief Mark a file as open in the program pool.
		 * 
		 * @param file_path The file which has been opened
		 */
		void openFile(const std::filesystem::path& file_path);

		/**
		 * @brief Mark a file as closed in the program pool.
		 *
		 * If the program pool discovers that all files for a particular program have been closed,
		 * it will remove the program from the pool.
		 * 
		 * @param file_path The file which has been closed
		 */
		void closeFile(const std::filesystem::path& file_path);

		void clean();
};
