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
#include <nlohmann/json.hpp>
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
#include "ThreadPool.h"

#include "../version.h"
#include "../updated_year.h"

static BashppServer server;

void signal_handler(int signum) {
	server.log_file << "Received signal " << signum << ", shutting down server." << std::endl;
	server.cleanup();
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

using json = nlohmann::json;

int main(int argc, char* argv[]) {
	// Trap signals
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	ThreadPool thread_pool(std::thread::hardware_concurrency());

	std::shared_ptr<std::istream> input_stream;
	std::shared_ptr<std::ostream> output_stream;

	server.log_file << "Language Server started with arguments: ";
	for (int i = 0; i < argc; ++i) {
		server.log_file << argv[i] << (i < argc - 1 ? " " : "");
	}
	server.log_file << std::endl;

	
	int client_fd = -1;
	int server_fd = -1;
	int socket_port = 0;

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
				server.log_file << "Using standard input/output for communication." << std::endl;
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
				std::cout << "Listening on TCP port " << socket_port << std::endl;
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
					std::cout << "Listening on Unix socket " << socket_path << std::endl;
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
	std::cout << "Client connected, running..." << std::endl;

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
		server.log_file << "Using standard input/output for communication." << std::endl;
	}

	server.setOutputStream(output_stream);

	std::streambuf* buffer = input_stream->rdbuf();

	while (true) {
		// Read headers
		std::string header;
		char c;
		while (buffer->sgetn(&c, 1) == 1 && c != '\n') {
			if (c == '\r') continue;
			header += c;
		}
		
		if (header.empty()) continue;

		server.log_file << "Received header: " << header << std::endl;

		size_t content_length = 0;
		if (header.find("Content-Length: ") == 0) {
			content_length = std::stoul(header.substr(16));
		}

		if (content_length == 0) continue;

		content_length += 2; // Account for the trailing "\r\n"

		// Read content
		std::vector<char> content(content_length);
		if (buffer->sgetn(content.data(), content_length) != content_length) {
			server.log_file << "Error reading content." << std::endl;
			break;
		}

		std::string message(content.begin(), content.end());
		server.log_file << "Received message (" << content_length << " bytes): " 
				 << message
				 << std::endl;

		// Grab a thread from the pool to process the message
		thread_pool.enqueue([message]() {
			try {
				server.processMessage(message);
			} catch (const std::exception& e) {
				server.log_file << "Error processing message: " << e.what() << std::endl;
			}
		});
	}

	server.log_file << "Language Server stopped." << std::endl;
	server.log_file.close();
	server.cleanup();
	return 0;
}
