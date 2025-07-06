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

using json = nlohmann::json;

// Bash++ Language Server
class BashppServer {
	private:
		static std::mutex cout_mutex; // Mutex for thread-safe output
		

		const std::unordered_map<std::string, std::function<json(const json&)>> handlers = {
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

		json shutdown(const json& params);

		void processMessage(const std::string& message);

		json handleInitialize(const json& params);
		json handleGotoDefinition(const json& params);
		json handleCompletion(const json& params);
		json handleHover(const json& params);
		json handleDocumentSymbol(const json& params);
		json handleDidOpen(const json& params);
		json handleDidChange(const json& params);
		json handleRename(const json& params);

};

#endif // SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_
