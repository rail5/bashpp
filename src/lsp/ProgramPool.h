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

class ProgramPool {
	private:
		size_t max_programs = 10; // Maximum number of programs to keep in the pool
		std::vector<std::shared_ptr<bpp::bpp_program>> programs;
		std::recursive_mutex pool_mutex; // Mutex to protect access to the pool

		std::unordered_map<std::string, size_t> program_indices; // Maps file paths to program indices in the pool

		std::unordered_map<std::string, bool> open_files; // Maps file paths to whether they are currently open

		bool utf16_mode = false; // Whether to use UTF-16 mode for character counting

		void _remove_oldest_program();
		void _remove_program(size_t index);
		std::shared_ptr<bpp::bpp_program> _parse_program(
			const std::string& file_path, 
			std::optional<std::pair<std::string, std::string>> replacement_file_contents = std::nullopt);

		// Configurable settings
		std::shared_ptr<std::vector<std::string>> include_paths = std::make_shared<std::vector<std::string>>();
		bool suppress_warnings = false;
	public:
		explicit ProgramPool(size_t max_programs = 10);

		void add_include_path(const std::string& path);
		void set_suppress_warnings(bool suppress);
		void set_utf16_mode(bool mode);
		bool get_utf16_mode() const;
		
		/**
		 * @brief Get or create a program for the given file path
		 * 
		 * If the program already exists in the pool, it returns that program.
		 * If the program does not exist, it creates a new program, adds it to the pool,
		 * and returns the new program.
		 */
		std::shared_ptr<bpp::bpp_program> get_program(const std::string& file_path, bool jump_queue = false);

		bool has_program(const std::string& file_path);

		std::shared_ptr<bpp::bpp_program> re_parse_program(const std::string& file_path);

		std::shared_ptr<bpp::bpp_program> re_parse_program(
			const std::string& file_path, 
			std::pair<std::string, std::string> replacement_file_contents);
		
		void open_file(const std::string& file_path);
		void close_file(const std::string& file_path);

		void clean();
};
