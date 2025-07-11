/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

// Bash++ Language Server

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <csignal>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <ext/stdio_filebuf.h> // Non-portable GNU extension

#include <unistd.h>
#include <getopt.h>

#include "BashppServer.h"

#include "../version.h"
#include "../updated_year.h"

static BashppServer server;
static int client_fd = -1;
static int server_fd = -1;
static int socket_port = 0;

void signal_handler(int signum) {
	server.log("Received signal: ", signum, ", cleaning up and exiting.");
	server.cleanup();
	if (client_fd != -1) {
		close(client_fd);
	}
	if (server_fd != -1) {
		close(server_fd);
	}
	exit(signum);
}

int setup_tcp_server(int port) {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "Error creating socket" << std::endl;
		return -1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(static_cast<uint16_t>(port));

	if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
		std::cerr << "Error binding socket" << std::endl;
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, 3) < 0) {
		std::cerr << "Error listening on socket" << std::endl;
		close(server_fd);
		return -1;
	}

	return server_fd;
}

int setup_unix_socket_server(const std::string& path) {
	int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "Error creating Unix socket" << std::endl;
		return -1;
	}

	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);
	address.sun_path[sizeof(address.sun_path) - 1] = '\0';

	unlink(path.c_str()); // Remove any existing socket file

	if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
		std::cerr << "Error binding Unix socket" << std::endl;
		close(server_fd);
		return -1;
	}

	if (listen(server_fd, 3) < 0) {
		std::cerr << "Error listening on Unix socket" << std::endl;
		close(server_fd);
		return -1;
	}

	return server_fd;
}

int main(int argc, char* argv[]) {
	// Trap signals
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	std::shared_ptr<std::istream> input_stream;
	std::shared_ptr<std::ostream> output_stream;

	server.log("Bash++ Language Server " bpp_compiler_version " starting...");
	std::string args;
	for (int i = 0; i < argc; ++i) {
		args += argv[i];
		if (i < argc - 1) {
			args += " ";
		}
	}
	server.log("Command line arguments: ", args);
	
	client_fd = -1;
	server_fd = -1;
	socket_port = 0;

	constexpr const char* help_string = "Bash++ Language Server " bpp_compiler_version "\n"
		"Usage: bpp-lsp [options]\n"
		"Options:\n"
		"  -h, --help              Show this help message\n"
		"  -v, --version           Show version information\n"
		"      --stdio             Use standard input/output for communication\n"
		"      --socket <port>     Use TCP socket for communication\n"
		"      --pipe <path>       Use Unix domain socket for communication\n";

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

	// getopt
	int opt;

	// Long options
	static struct option long_options[] = {
		{"help", no_argument, nullptr, 'h'},
		{"version", no_argument, nullptr, 'v'},
		{"stdio", no_argument, nullptr, 10000},
		{"socket", required_argument, nullptr, 10001},
		{"pipe", required_argument, nullptr, 10002},
		{nullptr, 0, nullptr, 0} // Sentinel
	};

	while ((opt = getopt_long(argc, argv, "hv", long_options, nullptr)) != -1) {
		switch (opt) {
			case 'h':
				std::cout << help_string << std::flush;
				return 0;
			case 'v':
				std::cout << version_string << std::flush;
				return 0;
			case 10000: // --stdio
				// Use standard input/output for communication
				server.log("Using standard input/output for communication.");
				input_stream = std::make_shared<std::istream>(std::cin.rdbuf());
				output_stream = std::make_shared<std::ostream>(std::cout.rdbuf());
				break;
			case 10001: // --socket
				// Use TCP socket for communication
				try {
					socket_port = std::stoi(optarg);
				} catch (...) {
					std::cerr << "Invalid port number: " << optarg << std::endl;
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
			case 10002: // --pipe
				// Use Unix domain socket for communication
				{
					std::string socket_path = optarg;
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
			default:
				std::cerr << "Unknown option: " << optopt << std::endl;
				return 1;
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
	if (client_fd != -1) {
		close(client_fd);
	}
	if (server_fd != -1) {
		close(server_fd);
	}
	return 0;
}
