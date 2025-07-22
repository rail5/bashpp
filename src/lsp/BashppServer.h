/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>
#include <optional>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include "ThreadPool.h"
#include "ProgramPool.h"

#include "static/Message.h"
#include "generated/ErrorCodes.h"

#include "generated/CompletionList.h"

#include "../bpp_include/bpp_codegen.h"

namespace bpp {

using json = nlohmann::json;

template<typename T>
void printValue(std::ostream& os, const T& value) {
	os << value;
}

template<typename... Ts>
void printValue(std::ostream& os, const std::variant<Ts...>& v) {
	std::visit([&os](auto&& arg) { os << arg; }, v);
}

// Bash++ Language Server
class BashppServer {
	private:
		pid_t pid = getpid();
		// Resources
		std::shared_ptr<std::istream> input_stream;
		std::shared_ptr<std::ostream> output_stream;
		std::optional<std::string> socket_path;
		ThreadPool thread_pool = ThreadPool(std::thread::hardware_concurrency());
		ProgramPool program_pool = ProgramPool(10); // Maximum 10 programs in the pool
		std::ofstream log_file;
		std::unordered_map<std::string, std::string> unsaved_changes; // Maps file paths to their unsaved contents
		std::mutex unsaved_changes_mutex;
		std::atomic<bool> stored_changes_content_updating = false;

		// Debouncing didChange notifications
		std::unordered_map<std::string, std::shared_ptr<std::atomic<uint64_t>>> debounce_timestamps;
		std::mutex debounce_mutex;

		void _sendMessage(const std::string& message);

		static std::mutex output_mutex; // Mutex for thread-safe output
		static std::mutex log_mutex; // Mutex for thread-safe logging
		
		// TODO(@rail5): When Debian 13 is released, use libfrozen-dev to make these maps constexpr
		const std::unordered_map<std::string, std::function<GenericResponseMessage(const GenericRequestMessage& )>> request_handlers = {
			{"initialize", std::bind(&BashppServer::handleInitialize, this, std::placeholders::_1)},
			{"textDocument/definition", std::bind(&BashppServer::handleDefinition, this, std::placeholders::_1)},
			{"textDocument/completion", std::bind(&BashppServer::handleCompletion, this, std::placeholders::_1)},
			{"textDocument/hover", std::bind(&BashppServer::handleHover, this, std::placeholders::_1)},
			{"textDocument/documentSymbol", std::bind(&BashppServer::handleDocumentSymbol, this, std::placeholders::_1)},
			{"textDocument/rename", std::bind(&BashppServer::handleRename, this, std::placeholders::_1)},
			{"shutdown", std::bind(&BashppServer::shutdown, this, std::placeholders::_1)}
		};

		const std::unordered_map<std::string, std::function<void(const GenericNotificationMessage& )>> notification_handlers = {
			{"textDocument/didOpen", std::bind(&BashppServer::handleDidOpen, this, std::placeholders::_1)},
			{"textDocument/didChange", std::bind(&BashppServer::handleDidChange, this, std::placeholders::_1)},
			{"workspace/didChangeWatchedFiles", std::bind(&BashppServer::handleDidChangeWatchedFiles, this, std::placeholders::_1)},
			{"textDocument/didClose", std::bind(&BashppServer::handleDidClose, this, std::placeholders::_1)}
		};

		static const GenericResponseMessage invalidRequestHandler(const GenericRequestMessage& request);
		static void invalidNotificationHandler(const GenericNotificationMessage& request);

		void processRequest(const GenericRequestMessage& request);
		void processNotification(const GenericNotificationMessage& notification);

		CompletionList default_completion_list;
	public:
		BashppServer();
		~BashppServer();

		void mainLoop();

		void setInputStream(std::shared_ptr<std::istream> stream);
		void setOutputStream(std::shared_ptr<std::ostream> stream);
		void setSocketPath(const std::string& path);
		void setLogFile(const std::string& path);

		GenericResponseMessage shutdown(const GenericRequestMessage& request);
		void cleanup();

		void processMessage(const std::string& message);

		// Request-Response handlers
		GenericResponseMessage handleInitialize(const GenericRequestMessage& request);
		GenericResponseMessage handleDefinition(const GenericRequestMessage& request);
		GenericResponseMessage handleCompletion(const GenericRequestMessage& request);
		GenericResponseMessage handleHover(const GenericRequestMessage& request);
		GenericResponseMessage handleDocumentSymbol(const GenericRequestMessage& request);
		GenericResponseMessage handleRename(const GenericRequestMessage& request);

		// Notification handlers
		void handleDidOpen(const GenericNotificationMessage& request);
		void handleDidChange(const GenericNotificationMessage& request);
		void handleDidChangeWatchedFiles(const GenericNotificationMessage& request);
		void handleDidClose(const GenericNotificationMessage& request);

		void sendResponse(const GenericResponseMessage& response);
		void sendNotification(const GenericNotificationMessage& notification);

		void publishDiagnostics(std::shared_ptr<bpp::bpp_program> program);

		void add_include_path(const std::string& path);
		void set_suppress_warnings(bool suppress);

		template <typename... Args>
		void log(Args&&... args) {
			if (!log_file.is_open()) {
				return; // Not logging
			}
			std::lock_guard<std::mutex> lock(log_mutex);
			log_file << "[" << std::to_string(pid) << "] ";
			((printValue(log_file, std::forward<Args>(args))), ...);
			log_file << std::endl;
		}
};

} // namespace bpp
