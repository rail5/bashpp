/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#ifndef SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_
#define SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_

#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <optional>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>

#include "ThreadPool.h"

#include "static/Message.h"

using json = nlohmann::json;

// Bash++ Language Server
class BashppServer {
	private:
		// Resources
		std::shared_ptr<std::istream> input_stream;
		std::shared_ptr<std::ostream> output_stream;
		std::optional<std::string> socket_path;
		ThreadPool thread_pool;

		static std::mutex output_mutex; // Mutex for thread-safe output
		
		const std::unordered_map<std::string, std::function<GenericResponseMessage(const GenericRequestMessage& )>> request_handlers = {
			{"initialize", std::bind(&BashppServer::handleInitialize, this, std::placeholders::_1)},
			{"textDocument/definition", std::bind(&BashppServer::handleGotoDefinition, this, std::placeholders::_1)},
			{"textDocument/completion", std::bind(&BashppServer::handleCompletion, this, std::placeholders::_1)},
			{"textDocument/hover", std::bind(&BashppServer::handleHover, this, std::placeholders::_1)},
			{"textDocument/documentSymbol", std::bind(&BashppServer::handleDocumentSymbol, this, std::placeholders::_1)},
			{"textDocument/rename", std::bind(&BashppServer::handleRename, this, std::placeholders::_1)},
			{"shutdown", std::bind(&BashppServer::shutdown, this, std::placeholders::_1)}
		};

		const std::unordered_map<std::string, std::function<void(const GenericNotificationMessage& )>> notification_handlers = {
			{"textDocument/didOpen", std::bind(&BashppServer::handleDidOpen, this, std::placeholders::_1)},
			{"textDocument/didChange", std::bind(&BashppServer::handleDidChange, this, std::placeholders::_1)}
		};

		static const GenericResponseMessage invalidRequestHandler(const GenericRequestMessage& request);
		static const void invalidNotificationHandler(const GenericNotificationMessage& request);

		void processRequest(const GenericRequestMessage& request);
		void processNotification(const GenericNotificationMessage& notification);
	public:
		std::ofstream log_file = std::ofstream("/tmp/lsp.log", std::ios::app);
		BashppServer();
		~BashppServer();

		void mainLoop();

		void setInputStream(std::shared_ptr<std::istream> stream);
		void setOutputStream(std::shared_ptr<std::ostream> stream);
		void setSocketPath(const std::string& path);

		GenericResponseMessage shutdown(const GenericRequestMessage& request);
		void cleanup();

		void processMessage(const std::string& message);

		// Request-Response handlers
		GenericResponseMessage handleInitialize(const GenericRequestMessage& request);
		GenericResponseMessage handleGotoDefinition(const GenericRequestMessage& request);
		GenericResponseMessage handleCompletion(const GenericRequestMessage& request);
		GenericResponseMessage handleHover(const GenericRequestMessage& request);
		GenericResponseMessage handleDocumentSymbol(const GenericRequestMessage& request);
		GenericResponseMessage handleRename(const GenericRequestMessage& request);

		// Notification handlers
		void handleDidOpen(const GenericNotificationMessage& request);
		void handleDidChange(const GenericNotificationMessage& request);

		void sendResponse(const GenericResponseMessage& response);
		void sendNotification(const GenericNotificationMessage& notification);

};

#endif // SRC_LANGUAGE_SERVER_BASHPP_SERVER_H_
