/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <fstream>
#include <antlr4-runtime.h>

#include "ProgramPool.h"

#include "../listener/BashppListener.h"
#include "../antlr/BashppLexer.h"
#include "../antlr/BashppParser.h"
#include "NullStream.h"

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

void ProgramPool::_remove_oldest_program() {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	if (programs.empty()) {
		return; // Nothing to remove
	}

	// Remove the oldest program (the first one in the vector)
	programs.erase(programs.begin());

	// Update the program indices
	std::vector<std::string> keys_to_remove;
	for (auto& entry : program_indices) {
		if (entry.second > 0) {
			entry.second--; // Decrement indices of all programs after the removed one
		} else {
			keys_to_remove.push_back(entry.first); // Mark for removal
		}
	}

	for (const auto& key : keys_to_remove) {
		program_indices.erase(key); // Remove the key from the map
	}
}

std::shared_ptr<bpp::bpp_program> ProgramPool::_parse_program(const std::string& file_path) {
	// Create output streams that will discard output
	std::shared_ptr<std::ostream> output_stream = std::make_shared<NullOStream>();
	std::shared_ptr<std::ostringstream> code_buffer = std::make_shared<NullOStringStream>();

	BashppListener listener;
	listener.set_source_file(file_path);
	listener.set_run_on_exit(false);
	listener.set_included(false);
	listener.set_output_stream(output_stream);
	listener.set_code_buffer(code_buffer);

	listener.set_suppress_warnings(suppress_warnings);
	listener.set_include_paths(include_paths);

	std::ifstream file_stream(file_path);
	antlr4::ANTLRInputStream input(file_stream);
	BashppLexer lexer(&input);
	antlr4::CommonTokenStream tokens(&lexer);
	tokens.fill();

	BashppParser parser(&tokens);
	parser.removeErrorListeners();
	std::unique_ptr<antlr4::DiagnosticErrorListener> error_listener = std::make_unique<antlr4::DiagnosticErrorListener>();
	parser.addErrorListener(error_listener.get());
	parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

	try {
		antlr4::tree::ParseTree* tree = parser.program();
		if (tree == nullptr) {
			throw std::runtime_error("Failed to parse program");
		}

		// Walk the tree
		antlr4::tree::ParseTreeWalker walker;
		walker.walk(&listener, tree);

		return listener.get_program();
	} catch (const std::exception& e) {
		std::cerr << "Error while parsing program: " << e.what() << std::endl;
		return nullptr; // Return nullptr if parsing fails
	}
}

std::shared_ptr<bpp::bpp_program> ProgramPool::get_program(const std::string& file_path) {
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
	return new_program;
}

bool ProgramPool::has_program(const std::string& file_path) {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	return program_indices.find(file_path) != program_indices.end();
}

void ProgramPool::re_parse_program(const std::string& file_path) {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);
	if (has_program(file_path)) {
		size_t index = program_indices[file_path];
		std::string main_source_file = programs[index]->get_main_source_file();
		programs[index] = _parse_program(main_source_file);
	} else {
		get_program(file_path); // If it doesn't exist, create it
	}
}

void ProgramPool::clean() {
	std::lock_guard<std::recursive_mutex> lock(pool_mutex);

	programs.clear();
	program_indices.clear();
}
