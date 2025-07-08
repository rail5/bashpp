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
#include "TypeRegistry.cpp"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <metaModel.json> <output_directory>" << std::endl;
		return 1;
	}
	std::string meta_model_path = argv[1];
	std::string output_directory = argv[2];
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
