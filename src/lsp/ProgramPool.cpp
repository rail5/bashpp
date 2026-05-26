/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <fstream>

#include "ProgramPool.h"

#include <AST/BashppParser.h>
#include <listener/BashppListener.h>

#include <include/NullStream.h>

#include <bpp_include/bpp_program.h>

ProgramPool::ProgramPool(size_t max_programs) : max_programs(max_programs) {
	// Initialize the program pool with a maximum number of programs
	programs.reserve(max_programs);
}

void ProgramPool::add_include_path(const std::string& path) {
	include_paths->push_back(path);
}

void ProgramPool::set_suppress_warnings(bool suppress) {
	suppress_warnings = suppress;
}

void ProgramPool::set_utf16_mode(bool mode) {
	utf16_mode = mode;
}

bool ProgramPool::get_utf16_mode() const {
	return utf16_mode;
}

void ProgramPool::set_target_bash_version(const BashVersion& version) {
	target_bash_version = version;
}

void ProgramPool::set_unsaved_file_contents(const std::string& file_path, const std::string& contents) {
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		unsaved_changes[file_path] = contents;
	}
	update_snapshot();
}

void ProgramPool::remove_unsaved_file_contents(const std::string& file_path) {
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		auto it = unsaved_changes.find(file_path);
		if (it != unsaved_changes.end()) {
			unsaved_changes.erase(it);
		}
	}
	update_snapshot();
}

std::string ProgramPool::get_file_contents(const std::string& file_path) {
	// If a copy exists in unsaved_changes, return that
	// Otherwise, read from the file on disk
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		auto it = unsaved_changes.find(file_path);
		if (it != unsaved_changes.end()) {
			return it->second;
		}
	}

	// Read from disk
	std::ifstream file_stream(file_path);
	if (!file_stream.is_open()) {
		return ""; // Could not open file
	}
	std::string contents((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
	return contents;
}

void ProgramPool::_remove_oldest_program() {
	_remove_program(0);
}

void ProgramPool::update_snapshot() {
	auto new_snapshot = std::make_unique<Snapshot>();
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		new_snapshot->programs_snapshot = programs;
		new_snapshot->program_indices_snapshot = program_indices;
		new_snapshot->open_files_snapshot = open_files;
		new_snapshot->unsaved_changes_snapshot = unsaved_changes;
	}
	{
		std::lock_guard<std::recursive_mutex> lock(snapshot_mutex);
		snapshot = std::move(new_snapshot);
	}
}

ProgramPool::Snapshot ProgramPool::load_snapshot() const {
	std::lock_guard<std::recursive_mutex> lock(snapshot_mutex);
	// Return a copy of the current snapshot
	return *snapshot;
}

void ProgramPool::_remove_program(size_t index) {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	if (index >= programs.size()) return; // Invalid index

	std::shared_ptr<bpp::bpp_program> program = programs[index];
	if (program != nullptr) {
		// Close all files associated with this program
		for (const auto& file_path : program->get_source_files()) {
			open_files.erase(file_path); // Remove the file from the open files map
		}
	}

	// Remove the program at the specified index
	programs.erase(programs.begin() + static_cast<std::vector<std::shared_ptr<bpp::bpp_program>>::difference_type>(index));

	// Update the program indices
	for (auto& [filePath, indices] : program_indices) {
		std::erase(indices, index); // Remove the index of the removed program

		std::transform(indices.begin(), indices.end(), indices.begin(), [index](size_t i) {
			return (i > index) ? (i - 1) : i; // Decrement indices of all programs after the removed one
		});
	}
	std::erase_if(program_indices, [](const auto& pair) { return pair.second.empty(); }); // Clean up any file paths with no associated programs
}

std::shared_ptr<bpp::bpp_program> ProgramPool::_parse_program(const std::string& file_path) {
	// Create output streams that will discard output
	std::shared_ptr<std::ostream> output_stream = std::make_shared<NullOStream>();
	std::shared_ptr<std::ostream> code_buffer = std::make_shared<NullOStream>();

	try {
		BashppListener listener;
		listener.set_source_file(file_path);
		listener.set_run_on_exit(false);
		listener.set_included(false);
		listener.set_output_stream(output_stream);
		listener.set_code_buffer(code_buffer);
		listener.set_suppress_warnings(suppress_warnings);
		listener.set_include_paths(include_paths);
		listener.set_target_bash_version(target_bash_version);
		listener.set_lsp_mode(true);
		listener.set_utf16_mode(utf16_mode);
		for (const auto& pair : unsaved_changes) {
			listener.set_replacement_file_contents(pair.first, pair.second);
		}

		AST::BashppParser parser;
		parser.setUTF16Mode(utf16_mode);

		if (unsaved_changes.contains(file_path)) {
			parser.setInputFromStringContents(unsaved_changes[file_path]);
		} else {
			parser.setInputFromFilePath(file_path);
		}

		auto program = parser.program();
		listener.set_parser_errors(parser.get_errors());
		if (program == nullptr) {
			program = std::make_shared<AST::Program>(); // Parsing failed
		}

		// Walk the tree
		listener.walk(program);
		return listener.get_program();
	} catch (const std::exception& e) {
		std::cerr << "Error while parsing program: " << e.what() << std::endl;
		return nullptr; // Return nullptr if parsing fails
	} catch (...) {
		std::cerr << "Unknown error while parsing program." << std::endl;
		return nullptr; // Return nullptr if parsing fails
	}
}

std::shared_ptr<bpp::bpp_program> ProgramPool::get_program(const std::string& file_path, bool jump_queue) {
	if (jump_queue) {
		// Jump the queue:
		// Skip getting a lock on the pool's main state, since we're not modifying it
		// Instead, read from the snapshot
		// And REFUSE to modify the pool if the file isn't already in it
		Snapshot snapshot_copy = load_snapshot();
		if (!snapshot_copy.program_indices_snapshot.contains(file_path)) return nullptr;

		// Many different programs may be associated with this file path
		// We just return the first one, which should be as good as any other
		// TODO(@rail5): Review this decision in the future
		const std::vector<size_t>& indices = snapshot_copy.program_indices_snapshot[file_path];
		if (indices.empty()) return nullptr; // No programs associated with this file path, shouldn't happen since we checked contains() but just in case
		return snapshot_copy.programs_snapshot[indices[0]];
	}

	std::shared_ptr<bpp::bpp_program> new_program = nullptr;
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);

		// Check if the program is already in the pool
		if (program_indices.contains(file_path)) {
			const std::vector<size_t>& indices = program_indices[file_path];
			if (!indices.empty()) return programs[indices[0]]; // Return the first program associated with this file path
		}

		// If the pool is full, remove the oldest program
		if (programs.size() >= max_programs) _remove_oldest_program();

		// Create a new program and add it to the pool
		new_program = _parse_program(file_path);
		if (new_program == nullptr) return nullptr; // Return nullptr if parsing fails

		programs.push_back(new_program);
		size_t index = programs.size() - 1;
		for (const auto& path : new_program->get_source_files()) {
			auto& indices_for_path = program_indices[path];
			if (!std::ranges::contains(indices_for_path, index)) {
				indices_for_path.push_back(index); // Map the file path to the program index
			}
		}

		open_files[file_path] = true; // Mark the file as open

		// If the main source file of any earlier programs is *included* in this program,
		// we can remove those earlier programs from the pool, effectively subsuming them
		// into this new program, since this new program's AST contains *all* of the
		// information in the earlier programs and more.
		for (size_t i = 0; i < programs.size() - 1; i++) {
			std::shared_ptr<bpp::bpp_program> program = programs[i];
			if (program == nullptr) continue;
			auto main_source_file = program->get_main_source_file();
			if (std::ranges::contains(new_program->get_source_files(), main_source_file) && main_source_file != file_path) {
				_remove_program(i);
				i--; // Decrement i to account for the removed program
			}
		}
	}

	update_snapshot();

	return new_program;
}

bool ProgramPool::has_program(const std::string& file_path) {
	// Scan the SNAPSHOT for the program
	Snapshot snapshot_copy = load_snapshot();
	const auto& program_indices_snapshot = snapshot_copy.program_indices_snapshot;
	return program_indices_snapshot.contains(file_path);
}

std::vector<std::shared_ptr<bpp::bpp_program>> ProgramPool::re_parse_programs(const std::string& file_path) {
	if (!has_program(file_path)) return { get_program(file_path) }; // If it doesn't exist, create it

	std::vector<std::shared_ptr<bpp::bpp_program>> successful_reparses;
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		// There are programs associated with this file, so we need to re-parse all of them
		std::vector<size_t> indices = program_indices[file_path];
		for (size_t index : indices) {
			std::string main_source_file = programs[index]->get_main_source_file();

			auto new_program = _parse_program(main_source_file);
			if (new_program != nullptr) {
				successful_reparses.push_back(new_program);
				programs[index] = new_program;

				// The new AST may reference a different set of source files than the old AST,
				// so we need to update the program_indices map accordingly
				// First, remove all references to this program index from all file paths
				for (auto& [filePath, indices] : program_indices) {
					std::erase(indices, index);
				}
				// Then, add references to this program index for all file paths referenced by the new AST
				for (const auto& path : new_program->get_source_files()) {
					auto& indices_for_path = program_indices[path];
					if (!std::ranges::contains(indices_for_path, index)) {
						indices_for_path.push_back(index); // Map the file path to the program index
					}
				}
				// Finally, clean up any file paths that no longer have any programs associated with them
				std::erase_if(program_indices, [](const auto& pair) { return pair.second.empty(); });
			}
		}
	}
	update_snapshot();

	return successful_reparses;
}

void ProgramPool::open_file(const std::string& file_path) {
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		auto it = program_indices.find(file_path);
		if (it == program_indices.end()) {
			// Invalid request to 'open file', ignore
			return;
		}

		open_files[file_path] = true; // Mark the file as open
	}
	update_snapshot();
}

void ProgramPool::close_file(const std::string& file_path) {
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);

		if (!open_files.contains(file_path)) return;

		open_files.erase(file_path); // Remove the file from the open files map

		// Now, find all of the programs associated with this file
		if (!program_indices.contains(file_path)) return;

		std::vector<size_t> indices = program_indices[file_path];
		std::sort(indices.rbegin(), indices.rend()); // Sort in reverse order so we can remove programs without invalidating indices of programs we haven't removed yet

		for (size_t index : indices) {
			// For each program:
			// Check if it's associated with any other files that are still open
			bool program_still_has_some_files_open = false;

			for (const auto& source_file : programs[index]->get_source_files()) {
				if (open_files.contains(source_file)) {
					program_still_has_some_files_open = true;
					break; // At least one file is still open, so we don't remove the program
				}
			}
			// If none of this program's files are still open,
			// we can remove it from the pool to free up memory
			if (!program_still_has_some_files_open) _remove_program(index);
		}
	}
	update_snapshot();
}

void ProgramPool::clean() {
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		programs.clear();
		program_indices.clear();
		open_files.clear();
	}
	update_snapshot(); // Update the snapshot after cleaning
}
