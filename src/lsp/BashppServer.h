/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include "ThreadPool.h"
#include "ProgramPool.h"

#include "static/Message.h"

#include "generated/CompletionList.h"
#include "generated/CompletionParams.h"

#include <entities/bpp_codegen.h>
#include <include/BashVersion.h>

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
	public:
		BashppServer() = default;
		~BashppServer() = default;
		BashppServer(const BashppServer& other) = delete; // Non-copyable
		BashppServer& operator=(const BashppServer& other) = delete;
		BashppServer(BashppServer&& other) noexcept = delete; // Non-movable
		BashppServer& operator=(BashppServer&& other) noexcept = delete;

		void mainLoop();

		void setLogFile(const std::string& path);
		void setTargetBashVersion(const BashVersion& version);
		void setThreadCount(size_t num_threads);

		GenericResponseMessage shutdown(const GenericRequestMessage& request);
		void exit(const GenericNotificationMessage& notification);
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
		void handleDidSave(const GenericNotificationMessage& request);

		void sendResponse(const GenericResponseMessage& response);
		void sendNotification(const GenericNotificationMessage& notification);

		void publishDiagnostics(std::shared_ptr<bpp::bpp_program> program);

		void add_include_path(const std::string& path);
		void set_suppress_warnings(bool suppress);

		template <typename... Args>
		void log(Args&&... args) {
			if (!log_file.is_open()) return; // Not logging

			std::lock_guard<std::mutex> lock(log_mutex);
			auto now = std::chrono::system_clock::now();
			auto now_time_t = std::chrono::system_clock::to_time_t(now);
			std::tm tm_buf{};
			localtime_r(&now_time_t, &tm_buf);
			log_file << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] ";
			log_file << "[" << std::to_string(pid) << "] ";
			((printValue(log_file, std::forward<Args>(args))), ...);
			log_file << std::endl;
		}

	private:
		pid_t pid = getpid();
		std::atomic<bool> exiting = false;
		std::atomic<bool> shutdown_requested = false;

		// Resources
		std::istream* input_stream = &std::cin;  // Held as std::stream* for future extensions beyond stdio
		std::ostream* output_stream = &std::cout;
		std::unique_ptr<ThreadPool> thread_pool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());
		ProgramPool program_pool = ProgramPool(10); // Maximum 10 programs in the pool
		std::ofstream log_file;

		// Debouncing didChange notifications
		struct DebounceState {
			std::atomic<uint64_t> change_generation{0};
			std::atomic<uint64_t> average_reparse_time_in_microseconds{50'000}; // 50ms initial guess
			std::atomic<uint32_t> debounce_time_in_milliseconds{100}; // Start with 100ms
		};

		// Map: program main URI -> DebounceState
		// Used for adaptive debouncing
		class DebounceStateMap {
			private:
				std::unordered_map<std::string, std::shared_ptr<DebounceState>> states;
				ProgramPool* pool;
				std::mutex map_mutex;

				void cleanup() {
					for (auto it = states.begin(); it != states.end(); ) {
						if (!pool->has_program(it->first)) {
							it = states.erase(it);
						} else {
							it++;
						}
					}
				}
			public:
				DebounceStateMap() = delete;
				explicit DebounceStateMap(ProgramPool* program_pool) : pool(program_pool) {}
				std::shared_ptr<DebounceState> get(const std::string& uri) {
					std::lock_guard<std::mutex> lock(map_mutex);
					cleanup(); // Clean up stale entries on every access
					auto it = states.find(uri);
					if (it != states.end()) return it->second;
					auto new_state = std::make_shared<DebounceState>();
					states[uri] = new_state;
					return new_state;
				}
		};
		DebounceStateMap debounce_states = DebounceStateMap(&program_pool);
		std::atomic<bool> processing_didChange{false};

		static std::string readHeaderLine(std::streambuf* buffer);

		void _sendMessage(const std::string& message);

		static std::mutex output_mutex; // Mutex for thread-safe output
		static std::mutex log_mutex; // Mutex for thread-safe logging
		
		static GenericResponseMessage invalidRequestHandler(const GenericRequestMessage& request);
		static void invalidNotificationHandler(const GenericNotificationMessage& request);

		void processRequest(const GenericRequestMessage& request);
		void processNotification(const GenericNotificationMessage& notification);

		CompletionList default_completion_list{
			.isIncomplete = false,
			.items = {
				CompletionItem{
					.label = "class",
					.kind = CompletionItemKind::Keyword,
					.detail = "Define a new class"
				},
				CompletionItem{
					.label = "public",
					.kind = CompletionItemKind::Keyword,
					.detail = "Public access specifier"
				},
				CompletionItem{
					.label = "private",
					.kind = CompletionItemKind::Keyword,
					.detail = "Private access specifier"
				},
				CompletionItem{
					.label = "protected",
					.kind = CompletionItemKind::Keyword,
					.detail = "Protected access specifier"
				},
				CompletionItem{
					.label = "method",
					.kind = CompletionItemKind::Keyword,
					.detail = "Define a new method in a class"
				},
				CompletionItem{
					.label = "constructor",
					.kind = CompletionItemKind::Keyword,
					.detail = "Define a constructor for a class"
				},
				CompletionItem{
					.label = "destructor",
					.kind = CompletionItemKind::Keyword,
					.detail = "Define a destructor for a class"
				},
				CompletionItem{
					.label = "virtual",
					.kind = CompletionItemKind::Keyword,
					.detail = "Declare a method to be virtual"
				},
				CompletionItem{
					.label = "this",
					.kind = CompletionItemKind::Keyword,
					.detail = "Reference the current object"
				},
				CompletionItem{
					.label = "super",
					.kind = CompletionItemKind::Keyword,
					.detail = "Reference the current object as an instance of its parent class"
				},
				CompletionItem{
					.label = "include",
					.kind = CompletionItemKind::Keyword,
					.detail = "Include a file"
				},
				CompletionItem{
					.label = "include_once",
					.kind = CompletionItemKind::Keyword,
					.detail = "Include a file only once"
				},
				CompletionItem{
					.label = "dynamic_cast",
					.kind = CompletionItemKind::Keyword,
					.detail = "Perform a dynamic cast"
				},
				CompletionItem{
					.label = "typeof",
					.kind = CompletionItemKind::Keyword,
					.detail = "Get the type of a pointer"
				},
				CompletionItem{
					.label = "new",
					.kind = CompletionItemKind::Keyword,
					.detail = "Create a new object"
				},
				CompletionItem{
					.label = "delete",
					.kind = CompletionItemKind::Keyword,
					.detail = "Delete an object"
				},
				CompletionItem{
					.label = "nullptr",
					.kind = CompletionItemKind::Keyword,
					.detail = "Null pointer constant"
				}
			}
		};

		using RequestHandler = GenericResponseMessage (BashppServer::*)(const GenericRequestMessage&);
		using NotificationHandler = void (BashppServer::*)(const GenericNotificationMessage&);

		struct RequestHandlerEntry {
			std::string_view method_name;
			RequestHandler handler;
		};
		struct NotificationHandlerEntry {
			std::string_view method_name;
			NotificationHandler handler;
		};

		/**
		 * @brief Maps request types to the functions that handle them.
		 * 
		 */
		static constexpr std::array<RequestHandlerEntry, 8> request_handlers = {{
			{"initialize",                  &BashppServer::handleInitialize},
			{"textDocument/definition",     &BashppServer::handleDefinition},
			{"textDocument/completion",     &BashppServer::handleCompletion},
			{"textDocument/hover",          &BashppServer::handleHover},
			{"textDocument/documentSymbol", &BashppServer::handleDocumentSymbol},
			{"textDocument/rename",         &BashppServer::handleRename},
			{"textDocument/references",     &BashppServer::handleReferences},
			{"shutdown",                    &BashppServer::shutdown}
		}};

		/**
		 * @brief Maps notification types to the functions that handle them.
		 * 
		 */
		static constexpr std::array<NotificationHandlerEntry, 6> notification_handlers = {{
			{"textDocument/didOpen",            &BashppServer::handleDidOpen},
			{"textDocument/didChange",          &BashppServer::handleDidChange},
			{"workspace/didChangeWatchedFiles", &BashppServer::handleDidChangeWatchedFiles},
			{"textDocument/didSave",            &BashppServer::handleDidSave},
			{"textDocument/didClose",           &BashppServer::handleDidClose},
			{"exit",                            &BashppServer::exit}
		}};
};

} // namespace bpp
