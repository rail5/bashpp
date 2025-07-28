/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <memory>
#include <getopt.h>
#include <unistd.h>
#include <algorithm>

#include "../version.h"
#include "../updated_year.h"

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

struct Option {
	const char shortopt;
	const char* longopt;
	const char* description;
	const bool takes_argument;

	constexpr Option(
		char shortopt,
		const char* longopt,
		const char* description,
		bool takes_argument)
		: shortopt(shortopt),
			longopt(longopt),
			description(description),
			takes_argument(takes_argument) {}
};

constexpr Option options[] = {
	{'o', "output", "Specify output file (default: run on exit)", true},
	{'s', "no-warnings", "Suppress warnings", false},
	{'I', "include", "Add directory to include path", true},
	{'p', "parse-tree", "Display parse tree (do not compile program)", false},
	{'t', "tokens", "Display tokens (do not compile program)", false},
	{'v', "version", "Display version information", false},
	{'h', "help", "Display this help message", false},
	{0, nullptr, nullptr, false} // Sentinel
};

constexpr size_t option_label_length(const Option& opt) {
	size_t length = 0;
	if (opt.shortopt) {
		length += 2; // '-x'
	}

	if (opt.shortopt && opt.longopt) {
		length += 2; // ', '
	}

	if (opt.longopt) {
		length += 2 + std::char_traits<char>::length(opt.longopt); // '--longopt'
	}

	if (opt.takes_argument) {
		length += 6; // ' <arg>'
	}
	
	return length;
}

constexpr size_t max_option_label_length(const Option* options) {
	size_t max_length = 0;
	for (size_t i = 0; options[i].shortopt || options[i].longopt; i++) {
		size_t length = option_label_length(options[i]);
		if (length > max_length) {
			max_length = length;
		}
	}

	return max_length;
}

std::string generate_help_string() {
	std::string help_string = "Usage: bpp [options] [file] ...\n"
		"If no file is specified, read from stdin\n"
		"All arguments after the file are passed to the compiled program\n"
		"Options:\n";
	
	for (const auto& opt : options) {
		if (!opt.shortopt && !opt.longopt) {
			break; // Sentinel reached
		}

		size_t label_length = option_label_length(opt);
		size_t padding_amount = max_option_label_length(options) - label_length;

		help_string += "  -" + std::string(1, opt.shortopt);
		if (opt.longopt) {
			help_string += ", --" + std::string(opt.longopt);
		}

		if (opt.takes_argument) {
			help_string += " <arg>";
		}

		help_string += std::string(padding_amount + 1, ' ') + opt.description + "\n";
	}

	return help_string;
}


struct Arguments {
	std::vector<char*> program_arguments;
	std::optional<std::string> input_file;
	std::optional<std::string> output_file;
	std::shared_ptr<std::vector<std::string>> include_paths = std::make_shared<std::vector<std::string>>();
	bool suppress_warnings = false;
	bool display_tokens = false;
	bool display_parse_tree = false;

	bool exit_early = false; // Exit early if the request is just -h/--help or -v/--version
};

std::pair<std::vector<char*>, std::vector<char*>> separate_compiler_and_program_options(Arguments* args, int argc, char* argv[]) {
	std::vector<char*> program_arguments;
	std::vector<char*> compiler_arguments;
	
	program_arguments.reserve(static_cast<std::vector<char*>::size_type>(argc));
	compiler_arguments.reserve(static_cast<std::vector<char*>::size_type>(argc));
	compiler_arguments.push_back(argv[0]);

	// Scan through each argument:
	// The first non-option argument should be interpreted as the source file to compile
	// Everything before it should be interpreted as arguments to the compiler
	// Everything after it should be interpreted as arguments to the compiled program
	
	std::vector<std::pair<char, const char*>> options_with_arguments;

	for (const auto& opt : options) {
		if (opt.takes_argument) {
			options_with_arguments.emplace_back(opt.shortopt, opt.longopt);
		}
	}

	bool expecting_argument = false;
	bool received_filename = false;
	for (int i = 1; i < argc; i++) {
		// If we've already received a filename, everything else is a program argument
		if (received_filename) {
			program_arguments.push_back(argv[i]);
			continue;
		}

		// If the last option we processed expected an argument,
		// Then this one is the argument for that option
		if (expecting_argument) {
			compiler_arguments.push_back(argv[i]);
			expecting_argument = false;
			continue;
		}

		// If this argument is an option, check if it takes an argument
		// If it does, set expecting_argument to true so we can process that next
		if (argv[i][0] == '-') {
			for (const auto& opt : options_with_arguments) {
				// Make sure we handle attached arguments to options, like -o- or --output=file.sh
				if (argv[i][1] == opt.first && argv[i][2] == '\0') {
					// Expect next opt to be the argument for this option
					expecting_argument = true;
				} else if (opt.second
					&& strcmp(argv[i] + 2, opt.second) == 0
					&& argv[i][2 + std::char_traits<char>::length(opt.second)] == '\0'
				) {
					// Expect next opt to be the argument for this option
					expecting_argument = true;
				}
			}
		}

		// Add the argument to the compiler_arguments array
		compiler_arguments.push_back(argv[i]);
		// If this argument is not an option, and we haven't received a filename yet,
		// Then this is the source file to compile
		if (!received_filename && argv[i][0] != '-') {
			args->input_file = argv[i];
			received_filename = true;
		}
	}

	return {compiler_arguments, program_arguments};
}

Arguments parse_arguments(int argc, char* argv[]) {
	Arguments args;

	args.include_paths->push_back("/usr/lib/bpp/stdlib/");

	auto [compiler_arguments, program_arguments] = separate_compiler_and_program_options(&args, argc, argv);
	args.program_arguments = std::move(program_arguments);
	
	// Getopt
	int c;
	opterr = 0;
	int option_index = 0;

	std::vector<char> options_with_arguments;

	std::string short_options;
	for (const auto& opt : options) {
		if (opt.shortopt) {
			short_options += opt.shortopt;
			if (opt.takes_argument) {
				short_options += ":";
				options_with_arguments.push_back(opt.shortopt);
			}
		}
	}
	short_options += '\0'; // Null-terminate the string

	// Long options
	static struct option long_options[(sizeof(options) / sizeof(options[0])) + 1];

	for (const auto& opt : options) {
		if (opt.longopt) {
			long_options[option_index++] = {opt.longopt, opt.takes_argument ? required_argument : no_argument, nullptr, opt.shortopt};
		}
	}
	long_options[option_index] = {nullptr, 0, nullptr, 0}; // Sentinel

	option_index = 0;

	bool received_output_filename = false;

	while (
		(c = getopt_long(
			static_cast<int>(compiler_arguments.size()),
			compiler_arguments.data(),
			short_options.c_str(),
			long_options,
			&option_index)
		) != -1
	) {
		switch (c) {
			case 'h':
				std::cout << program_name << " " << bpp_compiler_version << std::endl << generate_help_string();
				args.exit_early = true;
				return args;
				break;
			case 'I':
				// Verify the given include path is a directory
				if (!std::filesystem::exists(optarg) || !std::filesystem::is_directory(optarg)) {
					throw std::runtime_error("Include path '" + std::string(optarg) + "' does not exist or is not a directory");
				}
				args.include_paths->push_back(std::string(optarg));
				break;
			case 'o':
				if (received_output_filename) {
					throw std::runtime_error("Multiple output files specified");
				}
				args.output_file = std::string(optarg);
				
				if (args.output_file == "-") {
					break;
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
			case '?':
				if (std::find(options_with_arguments.begin(), options_with_arguments.end(), optopt) != options_with_arguments.end()) {
					throw std::runtime_error(std::string("Option -") + static_cast<char>(optopt) + " requires an argument\nUse -h for help");
				} else {
					throw std::runtime_error("Unknown option: -" + std::string(1, optopt));
				}
				break;
		}
	}

	return args;
}
