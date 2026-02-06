/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

// Bash++ Language Server

#include <iostream>
#include <string>
#include <memory>
#include <csignal>

#include <ext/stdio_filebuf.h> // Non-portable GNU extension

#include <unistd.h>

#include <lsp/BashppServer.h>
#include <lsp/helpers.h>

#include <include/BashVersion.h>
#include <include/xgetopt.h>

#include <version.h>
#include <updated_year.h>

bpp::BashppServer* p_server;
int client_fd = -1;
int server_fd = -1;
int socket_port = 0;

#include <include/exit_code.h>
volatile int bpp_exit_code = 0;

int main(int argc, char* argv[]) {
	// Trap signals
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	std::shared_ptr<std::istream> input_stream;
	std::shared_ptr<std::ostream> output_stream;

	bpp::BashppServer server;
	p_server = &server;

	server.log("Bash++ Language Server " bpp_compiler_version " starting...");
	std::string args;
	for (int i = 0; i < argc; ++i) {
		args += argv[i];
		if (i < argc - 1) {
			args += " ";
		}
	}
	server.log("Command line arguments: ", args);

	constexpr auto OptionParser = XGETOPT_PARSER(
		XGETOPT_OPTION('h', "help", "Show this help message", XGetOpt::NoArgument),
		XGETOPT_OPTION('v', "version", "Show version information", XGetOpt::NoArgument),
		XGETOPT_OPTION('l', "log", "Log messages to the specified file", XGetOpt::RequiredArgument, "file"),
		XGETOPT_OPTION('s', "no-warnings", "Suppress warnings", XGetOpt::NoArgument),
		XGETOPT_OPTION('b', "target-bash", "Set target Bash version (e.g., 5.2)", XGetOpt::RequiredArgument, "version"),
		XGETOPT_OPTION('I', "include", "Add a directory to include path", XGetOpt::RequiredArgument, "path"),
		XGETOPT_OPTION(1001, "stdio", "Use standard input/output for communication (default)", XGetOpt::NoArgument),
		XGETOPT_OPTION(1002, "port", "Use TCP port for communication", XGetOpt::RequiredArgument, "port"),
		XGETOPT_OPTION(1003, "socket", "Use Unix domain socket for communication", XGetOpt::RequiredArgument, "path")
	);
	
	constexpr const char* help_intro = "Bash++ Language Server " bpp_compiler_version "\n"
		"Usage: bpp-lsp [options]\n";

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
				{
					std::istringstream version_stream(std::string(arg.getArgument()));
					uint16_t major, minor;
					char dot;
					if (!(version_stream >> major >> dot >> minor) || dot != '.') {
						std::cerr << "Invalid Bash version format: " << std::string(arg.getArgument())
							<< std::endl << "Expected format: <major>.<minor> (e.g., 5.2)" << std::endl;
						return 1;
					}
					server.setTargetBashVersion(BashVersion{major, minor});
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
			case 10000: // --stdio
				// Use standard input/output for communication
				server.log("Using standard input/output for communication.");
				input_stream = std::make_shared<std::istream>(std::cin.rdbuf());
				output_stream = std::make_shared<std::ostream>(std::cout.rdbuf());
				break;
			case 10001: // --port
				// Use TCP port for communication
				try {
					socket_port = std::stoi(std::string(arg.getArgument()));
				} catch (...) {
					std::cerr << "Invalid port number: " << arg.getArgument() << std::endl;
					return 1;
				}
				server_fd = setup_tcp_server(socket_port);
				if (server_fd < 0) {
					return 1;
				}
				std::cerr << "Listening on TCP port " << socket_port << std::endl;
				server.log("Using TCP socket for communication on port ", socket_port);
				client_fd = accept(server_fd, nullptr, nullptr);
				if (client_fd < 0) {
					std::cerr << "Error accepting connection" << std::endl;
					close(server_fd);
					return 1;
				}
				break;
			case 10002: // --socket
				// Use Unix domain socket for communication
				{
					std::string socket_path(arg.getArgument());
					server_fd = setup_unix_socket_server(socket_path);
					server.setSocketPath(socket_path);
					if (server_fd < 0) {
						return 1;
					}
					std::cerr << "Listening on Unix socket " << socket_path << std::endl;
					server.log("Using Unix domain socket for communication at ", socket_path);
					client_fd = accept(server_fd, nullptr, nullptr);
					if (client_fd < 0) {
						std::cerr << "Error accepting connection" << std::endl;
						close(server_fd);
						return 1;
					}
				}
				break;
		}
	}
	std::cerr << "Client connected, running..." << std::endl;

	if (client_fd != -1) {
		auto filebuf = std::make_shared<__gnu_cxx::stdio_filebuf<char>>(client_fd, std::ios::in | std::ios::out);
		if (!filebuf->is_open()) {
			std::cerr << "Error opening filebuf for client_fd" << std::endl;
			close(client_fd);
			close(server_fd);
			return 1;
		}
		input_stream = std::make_shared<std::istream>(filebuf.get());
		output_stream = std::make_shared<std::ostream>(filebuf.get());
	}

	if (!input_stream || !output_stream) {
		// Default to stdio if no other method is specified
		input_stream = std::make_shared<std::istream>(std::cin.rdbuf());
		output_stream = std::make_shared<std::ostream>(std::cout.rdbuf());
		server.log("Defaulting to standard input/output for communication.");
	}

	server.setInputStream(input_stream);
	server.setOutputStream(output_stream);

	server.mainLoop();
	server.cleanup();
	p_server = nullptr;
	if (client_fd != -1) {
		close(client_fd);
	}
	if (server_fd != -1) {
		close(server_fd);
	}
	return 0;
}
