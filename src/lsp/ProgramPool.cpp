/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "ProgramPool.h"

typedef void* yyscan_t;

struct LexerExtra;

extern int yylex_init(yyscan_t* scanner);
extern int yylex_destroy(yyscan_t scanner);
extern void yyset_in(FILE* in_str, yyscan_t scanner);

extern void initLexer(yyscan_t yyscanner);
extern void destroyLexer(yyscan_t yyscanner);
extern void set_utf16_mode(bool enable, yyscan_t yyscanner);

#include "../flexbison/parser.tab.hpp"
#include "../flexbison/lex.yy.hpp"
#include "../listener/BashppListener.h"

#include "include/NullStream.h"

ProgramPool::ProgramPool(size_t max_programs) : max_programs(max_programs) {
	// Initialize the program pool with a maximum number of programs
	programs.reserve(max_programs);

	// Set default settings
	this->include_paths->push_back("/usr/lib/bpp/stdlib/");
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

void ProgramPool::_remove_oldest_program() {
	_remove_program(0);
}

void ProgramPool::update_snapshot() {
	auto new_snapshot = std::make_shared<Snapshot>();
	{
		std::lock_guard<std::recursive_mutex> lock(pool_mutex);
		new_snapshot->programs_snapshot = programs;
		new_snapshot->program_indices_snapshot = program_indices;
		new_snapshot->open_files_snapshot = open_files;
	}
	snapshot.store(new_snapshot, std::memory_order_release);
}

ProgramPool::Snapshot ProgramPool::load_snapshot() const {
	auto current_snapshot = snapshot.load(std::memory_order_acquire);
	if (current_snapshot == nullptr) {
		return ProgramPool::Snapshot{}; // return a default-constructed copy
	}
	return *current_snapshot; // Return a **copy** of the pointed-to Snapshot
}

void ProgramPool::_remove_program(size_t index) {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	if (index >= programs.size()) {
		return; // Invalid index
	}

	std::shared_ptr<bpp::bpp_program> program = programs[index];
	if (program != nullptr) {
		// Close all files associated with this program
		for (const auto& file_path : program->get_source_files()) {
			open_files.erase(file_path); // Remove the file from the open files map
		}
	}

	// Remove the program at the specified index
	programs.erase(programs.begin() + static_cast<std::vector<std::shared_ptr<bpp::bpp_program>>::difference_type>(index));

	std::vector<std::string> keys_to_remove;

	// Update the program indices
	for (auto& entry : program_indices) {
		if (entry.second > index) {
			entry.second--; // Decrement indices of all programs after the removed one
		} else if (entry.second == index) {
			keys_to_remove.push_back(entry.first); // Mark for removal
		}
	}

	for (const auto& key : keys_to_remove) {
		program_indices.erase(key); // Remove the key from the map
	}

	update_snapshot(); // Update the snapshot after removing the program
}

std::shared_ptr<bpp::bpp_program> ProgramPool::_parse_program(
	const std::string& file_path, 
	std::optional<std::pair<std::string, std::string>> replacement_file_contents
) {
	// Create output streams that will discard output
	std::shared_ptr<std::ostream> output_stream = std::make_shared<NullOStream>();
	std::shared_ptr<std::ostringstream> code_buffer = std::make_shared<NullOStringStream>();

	try {
		BashppListener listener;
		listener.set_source_file(file_path);
		listener.set_run_on_exit(false);
		listener.set_included(false);
		listener.set_output_stream(output_stream);
		listener.set_code_buffer(code_buffer);

		listener.set_suppress_warnings(suppress_warnings);
		listener.set_include_paths(include_paths);

		if (replacement_file_contents.has_value()) {
			listener.set_replacement_file_contents(replacement_file_contents->first, replacement_file_contents->second);
		}

		yyscan_t lexer;
		if (yylex_init(&lexer) != 0) {
			throw std::runtime_error("Could not initialize lexer");
		}
		FILE* input_file = fopen(file_path.c_str(), "r");
		if (input_file == nullptr) {
			throw std::runtime_error("Could not open source file: " + file_path);
		}
		yyset_in(input_file, lexer);
		initLexer(lexer);
		::set_utf16_mode(utf16_mode, lexer);
		std::shared_ptr<AST::Program> program = nullptr;
		yy::parser parser(program, lexer);
		int parse_result = parser.parse();
		fclose(input_file);
		destroyLexer(lexer);

		if (parse_result != 0 || program == nullptr) {
			throw std::runtime_error("Failed to parse program");
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
		auto it = snapshot_copy.program_indices_snapshot.find(file_path);
		if (it != snapshot_copy.program_indices_snapshot.end()) {
			return snapshot_copy.programs_snapshot[it->second];
		} else {
			return nullptr;
		}
	}

	std::lock_guard<std::recursive_mutex> lock(pool_mutex);

	// Check if the program is already in the pool
	auto it = program_indices.find(file_path);
	if (it != program_indices.end()) {
		return programs[it->second];
	}

	// If the pool is full, remove the oldest program
	if (programs.size() >= max_programs) {
		_remove_oldest_program();
	}

	// Create a new program and add it to the pool
	std::shared_ptr<bpp::bpp_program> new_program = _parse_program(file_path);
	if (new_program == nullptr) {
		return nullptr; // Return nullptr if parsing fails
	}

	programs.push_back(new_program);
	size_t index = programs.size() - 1;
	for (const auto& path : new_program->get_source_files()) {
		program_indices[path] = index; // Map the file path to the program index
	}

	open_files[file_path] = true; // Mark the file as open

	update_snapshot();

	return new_program;
}

bool ProgramPool::has_program(const std::string& file_path) {
	// Scan the SNAPSHOT for the program
	Snapshot snapshot_copy = load_snapshot();
	const auto& program_indices_snapshot = snapshot_copy.program_indices_snapshot;
	return program_indices_snapshot.find(file_path) != program_indices_snapshot.end();
}

std::shared_ptr<bpp::bpp_program> ProgramPool::re_parse_program(const std::string& file_path) {
	if (!has_program(file_path)) {
		return get_program(file_path); // If it doesn't exist, create it
	}

	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	// The program exists, so we can re-parse it
	size_t index = program_indices[file_path];
	std::string main_source_file = programs[index]->get_main_source_file();

	auto new_program = _parse_program(main_source_file);
	if (new_program == nullptr) {
		return nullptr; // Return nullptr if parsing fails
	}

	programs[index] = new_program;
	open_files[file_path] = true; // Mark the file as open

	update_snapshot();

	return programs[index];
}

std::shared_ptr<bpp::bpp_program> ProgramPool::re_parse_program(
			const std::string& file_path, 
			std::pair<std::string, std::string> replacement_file_contents
) {
	if (!has_program(file_path)) {
		return nullptr;
	}

	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	size_t index = program_indices[file_path];
	std::string main_source_file = programs[index]->get_main_source_file();

	auto new_program = _parse_program(main_source_file, replacement_file_contents);

	if (new_program == nullptr) {
		return nullptr; // Return nullptr if parsing fails
	}

	programs[index] = new_program;
	open_files[file_path] = true; // Mark the file as open

	update_snapshot();

	return programs[index];
}

void ProgramPool::open_file(const std::string& file_path) {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	
	auto it = program_indices.find(file_path);
	if (it == program_indices.end()) {
		// Invalid request to 'open file', ignore
		return;
	}

	open_files[file_path] = true; // Mark the file as open

	update_snapshot();
}

void ProgramPool::close_file(const std::string& file_path) {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	
	auto it = open_files.find(file_path);
	if (it != open_files.end()) {
		open_files.erase(it); // Remove the file from the open files map

		auto program_it = program_indices.find(file_path);
		if (program_it != program_indices.end()) {
			size_t index = program_it->second;
			bool program_still_has_some_files_open = false;
			for (const auto& source_file : programs[index]->get_source_files()) {
				if (open_files.find(source_file) != open_files.end()) {
					program_still_has_some_files_open = true;
					break; // At least one file is still open, so we don't remove the program
				}
			}

			if (!program_still_has_some_files_open) {
				// If no files are open, remove the program from the pool, free some memory
				_remove_program(index);
			}
		}

		update_snapshot();
	}
}

void ProgramPool::clean() {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);

	programs.clear();
	program_indices.clear();
	open_files.clear();

	update_snapshot(); // Update the snapshot after cleaning
}
