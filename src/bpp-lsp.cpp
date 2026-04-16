/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

// Bash++ Language Server

#include <iostream>
#include <string>
#include <csignal>

#include <unistd.h>

#include <lsp/BashppServer.h>
#include <include/BashVersion.h>
#include <include/xgetopt.h>

#include <version.h>
#include <updated_year.h>

int main(int argc, char* argv[]) {
	bpp::BashppServer server;

	constexpr XGetOpt::OptionParser<
		XGetOpt::Option<'h', "help", "Show this help message", XGetOpt::NoArgument>,
		XGetOpt::Option<'v', "version", "Show version information", XGetOpt::NoArgument>,
		XGetOpt::Option<'l', "log", "Log messages to the specified file", XGetOpt::RequiredArgument, "file">,
		XGetOpt::Option<'s', "no-warnings", "Suppress warnings", XGetOpt::NoArgument>,
		XGetOpt::Option<'b', "target-bash", "Set target Bash version (e.g., 5.2)", XGetOpt::RequiredArgument, "version">,
		XGetOpt::Option<'I', "include", "Add a directory to include path", XGetOpt::RequiredArgument, "path">,
		XGetOpt::Option<1001, "stdio", "Use standard input/output for communication (default)", XGetOpt::NoArgument>
	> OptionParser;
	
	constexpr const char* help_intro = "Bash++ Language Server " bpp_compiler_version "\n"
		"Usage: bpp-lsp [options]\n"
		"This program should be used via an editor plugin rather than invoked directly.\n"
		"Options:\n";

	constexpr const char* version_string = "Bash++ Language Server " bpp_compiler_version "\n"
		"Copyright (C) 2024-" bpp_compiler_updated_year " Andrew S. Rightenburg\n"
		"\n"
		"This program is free software; you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation; either version 3 of the License, or\n"
		"(at your option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program. If not, see http://www.gnu.org/licenses/.\n";


	XGetOpt::OptionSequence parsed_options;
	try {
		parsed_options = OptionParser.parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << "bpp-lsp: Error parsing arguments: " << e.what() << std::endl;
		return 1;
	}

	for (const auto& arg : parsed_options) {
		switch (arg.getShortOpt()) {
			default:
				// This should never happen since XGetOpt should throw on unrecognized options
				std::cerr << "bpp-lsp: Warning: Unhandled option '" << static_cast<char>(arg.getShortOpt()) << "'" << std::endl;
				break;
			case 'h':
				std::cout << help_intro << OptionParser.getHelpString();
				return 0;
			case 'I':
				server.add_include_path(std::string(arg.getArgument()));
				break;
			case 's':
				server.set_suppress_warnings(true);
				break;
			case 'b':
				try {
					server.setTargetBashVersion(BashVersion(arg.getArgument()));
				} catch (const std::exception& e) {
					std::cerr << "Invalid Bash version: " << arg.getArgument() << std::endl
						<< "Expected format: <major>[.<minor>] (e.g., 5.2)" << std::endl;
					return 1;
				}
				break;
			case 'v':
				std::cout << version_string << std::flush;
				return 0;
			case 'l':
				try {
					server.setLogFile(std::string(arg.getArgument()));
				} catch (const std::exception& e) {
					std::cerr << "Error setting log file: " << e.what() << std::endl;
					return 1;
				}
				break;
			case 1001: // --stdio
				// No-op
				// The language server does not yet support any other communication method
				break;
		}
	}
	std::cerr << "Client connected, running..." << std::endl;
	server.log("Defaulting to standard input/output for communication.");

	server.mainLoop();
	server.cleanup();
	return 0;
}
