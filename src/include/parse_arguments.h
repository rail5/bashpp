/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <optional>
#include <filesystem>
#include <memory>
#include <unistd.h>
#include <algorithm>

#include <include/FixedString.h>
#include <include/BashVersion.h>
#include <include/xgetopt.h>

#include <version.h>
#include <updated_year.h>

constexpr const char* program_name = "Bash++";

constexpr const char* copyright = "Copyright (C) 2024-"
	bpp_compiler_updated_year
	" Andrew S. Rightenburg\n\n"
	"This program is free software; you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation; either version 3 of the License, or\n"
	"(at your option) any later version.\n\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program. If not, see http://www.gnu.org/licenses/.\n";

constexpr const char* help_intro = 
	"Usage: bpp [options] [file] ...\n"
	"If no file is specified, read from stdin\n"
	"All arguments after the file are passed to the compiled program\n"
	"Options:\n";

constexpr XGetOpt::OptionParser<
	XGetOpt::Option<'o', "output", "Specify output file (default: run on exit)", XGetOpt::RequiredArgument>,
	XGetOpt::Option<'b', "target-bash", "Target Bash version (default: 5.2)", XGetOpt::RequiredArgument>,
	XGetOpt::Option<'s', "no-warnings", "Suppress warnings", XGetOpt::NoArgument>,
	XGetOpt::Option<'I', "include", "Add directory to include path", XGetOpt::RequiredArgument>,
	XGetOpt::Option<'t', "tokens", "Display tokens from lexer (do not compile program)", XGetOpt::NoArgument>,
	XGetOpt::Option<'p', "parse-tree", "Display parse tree (do not compile program)", XGetOpt::NoArgument>,
	XGetOpt::Option<'v', "version", "Display version information and exit", XGetOpt::NoArgument>,
	XGetOpt::Option<'h', "help", "Display this help message and exit", XGetOpt::NoArgument>
> OptionParser;

/**
 * @struct Arguments
 * @brief Represents the parsed command-line arguments given to the compiler
 *
 * This struct holds the parsed command-line arguments for the Bash++ compiler.
 * It includes options for input/output files, target Bash version, include paths,
 * warning suppression, and flags for displaying tokens or parse trees.
 * 
 */
struct Arguments {
	std::vector<char*> program_arguments;
	std::optional<std::string> input_file;
	std::optional<std::string> output_file;
	BashVersion target_bash_version = {5, 2}; // Default to Bash 5.2
	std::shared_ptr<std::vector<std::string>> include_paths = std::make_shared<std::vector<std::string>>();
	bool suppress_warnings = false;
	bool display_tokens = false;
	bool display_parse_tree = false;

	bool exit_early = false; // Exit early if the request is just -h/--help or -v/--version
};

inline Arguments parse_arguments(int argc, char* argv[]) {
	Arguments args;

	args.include_paths->push_back("/usr/lib/bpp/stdlib/");

	// Will throw if invalid arguments are provided
	auto [compiler_arguments, program_arguments]
		= OptionParser.parse_until<XGetOpt::StopCondition::AfterFirstNonOptionArgument>(argc, argv);

	args.program_arguments = std::vector<char*>{program_arguments.argv, program_arguments.argv + program_arguments.argc};

	if (compiler_arguments.getNonOptionArguments().size() > 0) {
		args.input_file = std::string(compiler_arguments.getNonOptionArguments()[0]);
	}

	bool received_output_filename = false;
	for (const auto& arg : compiler_arguments) {
		switch (arg.getShortOpt()) {
			case 'b':
				// Parse the target Bash version
				{
					std::istringstream version_stream(std::string(arg.getArgument()));
					uint16_t major, minor;
					char dot;
					if (!(version_stream >> major >> dot >> minor) || dot != '.') {
						throw std::runtime_error("Invalid Bash version format: " + std::string(arg.getArgument()) +
							"\nExpected format: <major>.<minor> (e.g., 5.2)");
					}
					args.target_bash_version = {major, minor};
				}
				break;
			case 'h':
				std::cout << program_name << " " << bpp_compiler_version << std::endl
					<< help_intro << OptionParser.getHelpString();
				args.exit_early = true;
				return args;
				break;
			case 'I':
				// Verify the given include path is a directory
				if (!std::filesystem::exists(arg.getArgument()) || !std::filesystem::is_directory(arg.getArgument())) {
					throw std::runtime_error("Include path '" + std::string(arg.getArgument()) + "' does not exist or is not a directory");
				}
				args.include_paths->push_back(std::string(arg.getArgument()));
				break;
			case 'o':
				if (received_output_filename) {
					throw std::runtime_error("Multiple output files specified");
				}

				if (std::string(arg.getArgument()) == "-") {
					args.output_file = arg.getArgument();
					break;
				}

				{
					std::filesystem::path output_path(arg.getArgument());
					if (output_path.is_absolute()) {
						args.output_file = output_path.string();
					} else {
						args.output_file = std::filesystem::current_path() / output_path;
					}
				}
			
				// Check if we have permission to write to the specified output file
				// If the file exists, verify write access; if it doesn't, verify write access on its parent directory
				try {
					if (std::filesystem::exists(args.output_file.value())) {
						if (!std::filesystem::is_regular_file(args.output_file.value())) {
							throw std::runtime_error("Output file '" + args.output_file.value() + "' is not a regular file");
						}
						if (access(args.output_file.value().c_str(), W_OK) != 0) {
							throw std::runtime_error("No write permission for output file '" + args.output_file.value() + "'");
						}
					} else {
						std::filesystem::path parent_path = std::filesystem::path(args.output_file.value()).parent_path();
						if (!std::filesystem::exists(parent_path) || !std::filesystem::is_directory(parent_path)) {
							throw std::runtime_error("Parent directory of output file '" + args.output_file.value() + "' does not exist or is not a directory");
						}
						if (access(parent_path.c_str(), W_OK) != 0) {
							throw std::runtime_error("No write permission for parent directory of output file '" + args.output_file.value() + "'");
						}
					}
				} catch (const std::exception& e) {
					throw std::runtime_error(std::string("Could not verify write permission for output file '") + args.output_file.value() + "': " + e.what());
				}
				break;
			case 'p':
				args.display_parse_tree = true;
				break;
			case 's':
				args.suppress_warnings = true;
				break;
			case 't':
				args.display_tokens = true;
				break;
			case 'v':
				std::cout << program_name << " " << bpp_compiler_version << std::endl << copyright;
				args.exit_early = true;
				return args;
				break;
		}
	}

	return args;
}
