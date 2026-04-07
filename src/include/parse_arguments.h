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
 * @class Arguments
 * @brief Represents the parsed command-line arguments given to the compiler
 *
 * This struct holds the parsed command-line arguments for the Bash++ compiler.
 * It includes options for input/output files, target Bash version, include paths,
 * warning suppression, and flags for displaying tokens or parse trees.
 * 
 */
class Arguments {
	private:
		std::vector<char*>                        m_program_arguments;
		std::optional<std::string>                m_input_file;
		std::optional<std::string>                m_output_file;
		BashVersion                               m_target_bash_version = {5, 2}; // Default to Bash 5.2
		std::shared_ptr<std::vector<std::string>> m_include_paths = std::make_shared<std::vector<std::string>>();
		bool f_suppress_warnings = false;
		bool f_display_tokens = false;
		bool f_display_parse_tree = false;
		bool f_exit_early = false; // Exit early if the request is just -h/--help or -v/--version

	public:
		/**
		 * @brief Sets the program arguments to be passed to the compiled program
		 *
		 * The program arguments are all of the arguments provided after the input file.
		 *   e.g.: `bpp -a -b -c source.bpp -x -y -z`
		 *   In this case, `-x -y -z` are the program arguments (passed to the compiled program)
		 *
		 * The argv pointers are expected to be borrowed from the original argv passed to main
		 * 
		 * @param argc The count of program arguments
		 * @param argv The array of program argument strings (borrowed from main's argv)
		 */
		void set_program_arguments(int argc, char** argv) {
			auto arguments_span = std::span<char*>{argv, static_cast<size_t>(argc)};
			this->m_program_arguments = std::vector<char*>(arguments_span.begin(), arguments_span.end());
		}
		const std::vector<char*>& program_arguments() const {
			return this->m_program_arguments;
		}

		/**
		 * @brief Sets the input file for the compiler
		 *
		 * If no input file is set, the compiler will read from stdin.
		 * The method verifies that the provided input file exists and is a regular file.
		 * 
		 * @param input_file The path to the input file to compile
		 * @throws std::runtime_error if the input file does not exist or is not a regular file
		 */
		void set_input_file(const std::string_view& input_file) {
			// Verify that the file exists and is a regular file
			if (!std::filesystem::exists(input_file)) {
				throw std::runtime_error("Source file '" + std::string(input_file) + "' does not exist");
			}
			if (!std::filesystem::is_regular_file(input_file)) {
				throw std::runtime_error("Source file '" + std::string(input_file) + "' is not a regular file");
			}
			this->m_input_file = input_file;
		}
		const std::optional<std::string>& input_file() const {
			return this->m_input_file;
		}

		/**
		* @brief Parses a given output_file argument and updates the Arguments object accordingly.
		*
		* If '-' is given, compiled code will be written to stdout.
		* Otherwise, the output file path is resolved and verified for write permissions.
		* Exceptions are thrown if the output file is invalid or not writable.
		* 
		* @param output_file_arg The output file argument string to parse
		* @throws std::runtime_error if the output file argument is invalid or not writable
		*/
		void set_output_file(const std::string_view& output_file_arg) {
			if (output_file_arg == "-") {
				this->m_output_file = output_file_arg;
				return;
			}

			std::string parsed_output_file;

			{
				std::filesystem::path output_path(output_file_arg);
				if (output_path.is_absolute()) {
					parsed_output_file = output_path.string();
				} else {
					parsed_output_file = std::filesystem::current_path() / output_path;
				}
			}

			// Check if we have permission to write to the specified output file
			// If the file exists, verify write access; if it doesn't, verify write access on its parent directory
			try {
				if (std::filesystem::exists(parsed_output_file)) {
					if (access(parsed_output_file.c_str(), W_OK) != 0) {
						throw std::runtime_error("No write permission for output file '" + parsed_output_file + "'");
					}
				} else {
					std::filesystem::path parent_path = std::filesystem::path(parsed_output_file).parent_path();
					if (!std::filesystem::exists(parent_path) || !std::filesystem::is_directory(parent_path)) {
						throw std::runtime_error("Parent directory of output file '" + parsed_output_file + "' does not exist or is not a directory");
					}
					if (access(parent_path.c_str(), W_OK) != 0) {
						throw std::runtime_error("No write permission for parent directory of output file '" + parsed_output_file + "'");
					}
				}
			} catch (const std::exception& e) {
				throw std::runtime_error(std::string("Could not verify write permission for output file '") + parsed_output_file + "': " + e.what());
			}

			this->m_output_file = parsed_output_file;
		}
		const std::optional<std::string>& output_file() const {
			return this->m_output_file;
		}

		/**
		 * @brief Parses a given target Bash version argument and updates the Arguments object accordingly.
		 *
		 * The version argument should be in the format "<major>.<minor>", e.g. "5.2".
		 * The method validates the format and updates the target_bash_version member.
		 * 
		 * @param version_arg The target Bash version argument string to parse
		 * @throws std::runtime_error if the version argument is in an invalid format
		 */
		void set_target_bash_version(const std::string_view& version_arg) {
					std::istringstream version_stream((std::string(version_arg)));
					uint16_t major, minor;
					char dot;
					if (!(version_stream >> major >> dot >> minor) || dot != '.') {
						throw std::runtime_error("Invalid Bash version format: " + std::string(version_arg) +
							"\nExpected format: <major>.<minor> (e.g., 5.2)");
					}
					this->m_target_bash_version = {major, minor};
		}
		const BashVersion& target_bash_version() const {
			return this->m_target_bash_version;
		}

		/**
		 * @brief Adds a directory to the include paths
		 *
		 * The include paths are directories that the compiler will search for included files.
		 * The last include path is always /usr/lib/bpp/stdlib/, which contains the standard library for Bash++.
		 * Additional include paths are searched in the order they are given, and all include paths are searched
		 *  before the standard library path, allowing users to override standard library files if needed.
		 * 
		 * @param path The directory path to add to the include paths
		 * @throws std::runtime_error if the provided path is not a directory
		 */
		void add_include_path(const std::string_view& path) {
			if (!std::filesystem::is_directory(path)) {
				throw std::runtime_error("Include path '" + std::string(path) + "' is not a directory");
			}
			this->m_include_paths->emplace_back(path);
		}
		const std::shared_ptr<std::vector<std::string>>& include_paths() const {
			return this->m_include_paths;
		}
		
		void set_suppress_warnings(bool suppress) {
			this->f_suppress_warnings = suppress;
		}
		bool suppress_warnings() const {
			return this->f_suppress_warnings;
		}

		void set_display_tokens(bool display) {
			this->f_display_tokens = display;
		}
		bool display_tokens() const {
			return this->f_display_tokens;
		}

		void set_display_parse_tree(bool display) {
			this->f_display_parse_tree = display;
		}
		bool display_parse_tree() const {
			return this->f_display_parse_tree;
		}

		void set_exit_early(bool exit) {
			this->f_exit_early = exit;
		}
		bool exit_early() const {
			return this->f_exit_early;
		}
};

inline Arguments parse_arguments(int argc, char* argv[]) {
	Arguments args;

	// Will throw if invalid arguments are provided
	auto [compiler_arguments, program_arguments]
		= OptionParser.parse_until<XGetOpt::StopCondition::AfterFirstNonOptionArgument>(argc, argv);

	args.set_program_arguments(program_arguments.argc, program_arguments.argv);

	if (compiler_arguments.getNonOptionArguments().size() > 0) {
		args.set_input_file(compiler_arguments.getNonOptionArguments()[0]);
	}

	for (const auto& arg : compiler_arguments) {
		switch (arg.getShortOpt()) {
			default:
				// This should never happen since XGetOpt should throw on unrecognized options
				std::cerr << program_name << ": Warning: Unhandled option '" << static_cast<char>(arg.getShortOpt()) << "'" << std::endl;
				break;
			case 'b':
				args.set_target_bash_version(arg.getArgument());
				break;
			case 'h':
				std::cout << program_name << " " << bpp_compiler_version << std::endl
					<< help_intro << OptionParser.getHelpString();
				args.set_exit_early(true);
				return args;
				break;
			case 'I':
				args.add_include_path(arg.getArgument());
				break;
			case 'o':
				args.set_output_file(arg.getArgument());
				break;
			case 'p':
				args.set_display_parse_tree(true);
				break;
			case 's':
				args.set_suppress_warnings(true);
				break;
			case 't':
				args.set_display_tokens(true);
				break;
			case 'v':
				std::cout << program_name << " " << bpp_compiler_version << std::endl << copyright;
				args.set_exit_early(true);
				return args;
				break;
		}
	}

	return args;
}
