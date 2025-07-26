/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "helpers.h"

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

	// Allow address reuse
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cerr << "Error setting SO_REUSEADDR" << std::endl;
		close(server_fd);
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
