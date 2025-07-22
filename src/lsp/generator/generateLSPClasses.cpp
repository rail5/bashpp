/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

/**
 * This file is part of the Bash++ Language Server Protocol implementation.
 * 
 * This is responsible for reading `metaModel.json`
 * and using it to automatically generate C++ classes for the LSP model,
 * which we can then use to implement the LSP server.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "TypeRegistry.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <metaModel.json> <output_directory>" << std::endl;
		return 1;
	}
	std::string meta_model_path = argv[1];

	// Validate the meta model file
	if (!std::filesystem::exists(meta_model_path)) {
		std::cerr << "Error: Meta model file '" << meta_model_path << "' does not exist." << std::endl;
		return 1;
	}

	// Verify it's a regular file
	if (!std::filesystem::is_regular_file(meta_model_path)) {
		std::cerr << "Error: Meta model file '" << meta_model_path << "' is not a regular file." << std::endl;
		return 1;
	}

	std::string output_directory = argv[2];

	// Validate output directory
	if (!std::filesystem::exists(output_directory)) {
		std::cerr << "Error: Output directory '" << output_directory << "' does not exist." << std::endl;
		return 1;
	}

	// Check if the output directory is writable
	if (!std::filesystem::is_directory(output_directory) ||
		((std::filesystem::status(output_directory).permissions() & std::filesystem::perms::owner_write) == std::filesystem::perms::none)) {
		std::cerr << "Error: Output directory '" << output_directory << "' is not writable." << std::endl;
		return 1;
	}

	// Load LSP meta model
	std::ifstream meta_file(meta_model_path);
	nlohmann::json meta_model = nlohmann::json::parse(meta_file);

	// Process and generate types
	TypeRegistry registry;
	registry.set_output_directory(output_directory);
	registry.load_meta_model(meta_model);
	registry.generate_all_types();

	return 0;
}
