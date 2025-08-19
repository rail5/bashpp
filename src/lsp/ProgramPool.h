/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>

#include "../bpp_include/bpp.h"

/**
 * @class ProgramPool
 * @brief Manages a pool of bpp_program objects for efficient reuse and access.
 *
 * Each parsed program has its data stored in a bpp_program object.
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
	private:
		size_t max_programs = 10; // Maximum number of programs to keep in the pool
		std::vector<std::shared_ptr<bpp::bpp_program>> programs;
		std::unordered_map<std::string, size_t> program_indices; // Maps file paths to program indices in the pool
		std::unordered_map<std::string, bool> open_files; // Maps file paths to whether they are currently open
		std::recursive_mutex pool_mutex; // Mutex to protect access to the pool

		// Pool snapshots:
		struct Snapshot {
			std::vector<std::shared_ptr<bpp::bpp_program>> programs_snapshot; // Snapshot of the current programs in the pool
			std::unordered_map<std::string, size_t> program_indices_snapshot; // Snapshot of the current program indices
			std::unordered_map<std::string, bool> open_files_snapshot; // Snapshot of the current open files
		};
		std::atomic<std::shared_ptr<Snapshot>> snapshot; // Atomic snapshot for thread-safe access

		bool utf16_mode = false; // Whether to use UTF-16 mode for character counting

		void _remove_oldest_program();
		void _remove_program(size_t index);
		std::shared_ptr<bpp::bpp_program> _parse_program(
			const std::string& file_path, 
			std::optional<std::pair<std::string, std::string>> replacement_file_contents = std::nullopt);

		// Configurable settings
		std::shared_ptr<std::vector<std::string>> include_paths = std::make_shared<std::vector<std::string>>();
		bool suppress_warnings = false;

		void update_snapshot();
		Snapshot load_snapshot() const;
	public:
		explicit ProgramPool(size_t max_programs = 10);

		/**
		 * @brief Add an include path for use by all future programs to be added to the pool.
		 *
		 * This mirrors the `-I` option in bpp
		 * 
		 * @param path The include path to add.
		 */
		void add_include_path(const std::string& path);
		void set_suppress_warnings(bool suppress);
		void set_utf16_mode(bool mode);
		bool get_utf16_mode() const;
		
		/**
		 * @brief Get or create a program for the given file path
		 * 
		 * If a program in the pool reports that it controls the file at the given path,
		 * that program is returned.
		 * If no program exists for that file, we create a new program, add it to the pool,
		 * and return the new program.
		 *
		 * @param file_path The source file path to get or create a program for.
		 * @param jump_queue Whether to jump the request queue. If true, the request is
		 *                processed immediately. However, we will also refuse to create
		*                 a new program -- if none exists, and you've asked to jump the
		*                 queue, we will simply return nullptr.
		 */
		std::shared_ptr<bpp::bpp_program> get_program(const std::string& file_path, bool jump_queue = false);

		/**
		 * @brief Check if a program for the given file path exists in the pool.
		 * 
		 * @param file_path The source file path to check for a program.
		 * @return true if a program exists in the pool for the given file path, false otherwise.
		 */
		bool has_program(const std::string& file_path);

		/**
		 * @brief Re-parse a program for the given file path.
		 * 
		 * @param file_path The source file which has been modified, triggering the re-parse.
		 * @return std::shared_ptr<bpp::bpp_program> The re-parsed program
		 */
		std::shared_ptr<bpp::bpp_program> re_parse_program(const std::string& file_path);

		/**
		 * @brief Re-parse a program for the given file path, with unsaved changes parsed instead of the file on disk.
		 * 
		 * @param file_path The source file which has been modified, triggering the re-parse.
		 * @param replacement_file_contents A pair: the first element is the path to the file with unsaved changes,
		 *                                  the second element is the new contents of the file to use instead of
		 *                                  the file on disk.
		 * @return std::shared_ptr<bpp::bpp_program> 
		 */
		std::shared_ptr<bpp::bpp_program> re_parse_program(
			const std::string& file_path, 
			std::pair<std::string, std::string> replacement_file_contents);
		
		/**
		 * @brief Mark a file as open in the program pool.
		 * 
		 * @param file_path The file which has been opened
		 */
		void open_file(const std::string& file_path);

		/**
		 * @brief Mark a file as closed in the program pool.
		 *
		 * If the program pool discovers that all files for a particular program have been closed,
		 * it will remove the program from the pool.
		 * 
		 * @param file_path The file which has been closed
		 */
		void close_file(const std::string& file_path);

		void clean();
};
