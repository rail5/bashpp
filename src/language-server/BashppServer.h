/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#ifndef SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_
#define SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_

#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>
#include "static/Message.h"

using json = nlohmann::json;

// Bash++ Language Server
class BashppServer {
	private:
		static std::mutex cout_mutex; // Mutex for thread-safe output
		

		const std::unordered_map<std::string, std::function<GenericResponseMessage(const GenericRequestMessage& )>> handlers = {
			{"initialize", std::bind(&BashppServer::handleInitialize, this, std::placeholders::_1)},
			{"textDocument/definition", std::bind(&BashppServer::handleGotoDefinition, this, std::placeholders::_1)},
			{"textDocument/completion", std::bind(&BashppServer::handleCompletion, this, std::placeholders::_1)},
			{"textDocument/hover", std::bind(&BashppServer::handleHover, this, std::placeholders::_1)},
			{"textDocument/documentSymbol", std::bind(&BashppServer::handleDocumentSymbol, this, std::placeholders::_1)},
			{"textDocument/didOpen", std::bind(&BashppServer::handleDidOpen, this, std::placeholders::_1)},
			{"textDocument/didChange", std::bind(&BashppServer::handleDidChange, this, std::placeholders::_1)},
			{"textDocument/rename", std::bind(&BashppServer::handleRename, this, std::placeholders::_1)},
			{"shutdown", std::bind(&BashppServer::shutdown, this, std::placeholders::_1)}
		};
	public:
		std::ofstream log_file = std::ofstream("/tmp/lsp.log", std::ios::app);
		BashppServer();
		~BashppServer();

		GenericResponseMessage shutdown(const GenericRequestMessage& request);

		void processMessage(const std::string& message);

		GenericResponseMessage handleInitialize(const GenericRequestMessage& request);
		GenericResponseMessage handleGotoDefinition(const GenericRequestMessage& request);
		GenericResponseMessage handleCompletion(const GenericRequestMessage& request);
		GenericResponseMessage handleHover(const GenericRequestMessage& request);
		GenericResponseMessage handleDocumentSymbol(const GenericRequestMessage& request);
		GenericResponseMessage handleDidOpen(const GenericRequestMessage& request);
		GenericResponseMessage handleDidChange(const GenericRequestMessage& request);
		GenericResponseMessage handleRename(const GenericRequestMessage& request);

};

#endif // SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_
