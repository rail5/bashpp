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

#include <frozen/string.h>
#include <frozen/unordered_map.h>

#include "ThreadPool.h"
#include "ProgramPool.h"

#include "static/Message.h"
#include "generated/ErrorCodes.h"

#include "generated/CompletionList.h"
#include "generated/CompletionParams.h"

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


/**
 * @class BashppServer
 * @brief The main server class for handling LSP requests and notifications.
 *
 * This class manages the server's lifecycle, handles incoming messages,
 * processes requests and notifications, logs, and maintains the state of the server.
 * 
 */
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
		
		using RequestHandler = GenericResponseMessage (BashppServer::*)(const GenericRequestMessage&);
		using NotificationHandler = void (BashppServer::*)(const GenericNotificationMessage&);
		/**
		 * @brief Maps request types to the functions that handle them.
		 * 
		 */
		static const frozen::unordered_map<frozen::string, RequestHandler, 8> request_handlers;

		/**
		 * @brief Maps notification types to the functions that handle them.
		 * 
		 */
		static const frozen::unordered_map<frozen::string, NotificationHandler, 4> notification_handlers;

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
		GenericResponseMessage handleHover(const GenericRequestMessage& request);
		GenericResponseMessage handleDocumentSymbol(const GenericRequestMessage& request);
		GenericResponseMessage handleRename(const GenericRequestMessage& request);
		GenericResponseMessage handleReferences(const GenericRequestMessage& request);
		GenericResponseMessage handleCompletion(const GenericRequestMessage& request);

		CompletionList handleATCompletion(const CompletionParams& params);
		CompletionList handleDOTCompletion(const CompletionParams& params);

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
