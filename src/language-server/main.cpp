/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

// Bash++ Language Server

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <thread>

#include "BashppServer.h"
#include "ThreadPool.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
	BashppServer server;
	ThreadPool thread_pool(std::thread::hardware_concurrency());

	server.log_file << "Language Server started with arguments: ";
	for (int i = 0; i < argc; ++i) {
		server.log_file << argv[i] << (i < argc - 1 ? " " : "");
	}
	server.log_file << std::endl;

	std::streambuf* buffer = std::cin.rdbuf();
	int request_id = -1;

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
		thread_pool.enqueue([&server, message]() {
			try {
				server.processMessage(message);
			} catch (const std::exception& e) {
				server.log_file << "Error processing message: " << e.what() << std::endl;
			}
		});
	}

	server.log_file << "Language Server stopped." << std::endl;
	server.log_file.close();
	return 0;
}
