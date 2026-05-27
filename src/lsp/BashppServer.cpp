/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "BashppServer.h"

#include <unistd.h>

#include "generated/PublishDiagnosticsNotification.h"
#include "generated/ErrorCodes.h"

#include <bpp_include/bpp_program.h>

std::mutex bpp::BashppServer::output_mutex;
std::mutex bpp::BashppServer::log_mutex;

void bpp::BashppServer::setLogFile(const std::string& path) {
	log_file.open(path, std::ios::app);
	if (!log_file.is_open()) throw std::runtime_error("Failed to open log file: " + path);
}

void bpp::BashppServer::setTargetBashVersion(const BashVersion& version) {
	program_pool.set_target_bash_version(version);
}

void bpp::BashppServer::setThreadCount(size_t num_threads) {
	if (thread_pool->isActive()) throw std::runtime_error("Cannot change thread count while the thread pool is active.");
	thread_pool = std::make_unique<ThreadPool>(num_threads);
	log("Set number of threads to: ", num_threads);
}

void bpp::BashppServer::cleanup() {
	if (exiting.exchange(true)) return; // Already exiting

	if (output_stream) output_stream->flush();

	thread_pool->cleanup();
	log("Bash++ Language Server cleaned up and exiting.");
	log_file.close();
}

std::string bpp::BashppServer::readHeaderLine(std::streambuf* buffer) {
	std::string header;
	char c;
	while (true) {
		const std::streamsize n = buffer->sgetn(&c, 1);
		if (n != 1) {
			throw std::runtime_error("End of input stream reached while reading header.");
		}
		if (c == '\n') break;
		if (c == '\r') continue;
		header += c;
	}
	return header;
}

void bpp::BashppServer::mainLoop() {
	if (!input_stream || !output_stream) {
		throw std::runtime_error("Input or output stream not set.");
	}

	log("Bash++ Language Server initialized.");
	log("Using ", thread_pool->getThreadCount(), " threads for processing requests.");

	std::streambuf* buffer = input_stream->rdbuf();
	while (!exiting) {
		std::string header;
		try {
			header = readHeaderLine(buffer);
		} catch (...) {
			log("End of input stream or error encountered.");
			break;
		}
		
		if (header.empty()) continue;

		log("Received header: ", header);

		size_t content_length = 0;
		if (header.starts_with("Content-Length: ")) {
			content_length = std::stoul(header.substr(16));
		}

		if (content_length == 0) continue;

		content_length += 2; // Account for the trailing "\r\n"

		// Read content
		std::vector<char> content(content_length);
		std::streamsize read_count = buffer->sgetn(content.data(), static_cast<std::streamsize>(content_length));
		if (read_count != static_cast<std::streamsize>(content_length)) {
			log("Error reading content, expected ", content_length, " bytes but got less.");
			break;
		}

		std::string message(content.begin(), content.end());
		log("Received message (", content_length, " bytes): ", message);

		// Grab a thread from the pool to process the message
		thread_pool->enqueue([this, message]() {
			try {
				this->processMessage(message);
			} catch (const std::exception& e) {
				log("Error processing message: ", e.what());
			}
		});
	}
	log("Main loop exiting.");
}

GenericResponseMessage bpp::BashppServer::invalidRequestHandler(const GenericRequestMessage& request) {
	throw std::runtime_error("Request not understood: " + request.method);
}

void bpp::BashppServer::invalidNotificationHandler(const GenericNotificationMessage& request) {
	throw std::runtime_error("Notification not understood: " + request.method);
}

void bpp::BashppServer::_sendMessage(const std::string& message) {
	std::lock_guard<std::mutex> lock(output_mutex);
	std::string header = "Content-Length: " + std::to_string(message.size()) + "\r\n\r\n";
	*output_stream << header << message << std::flush;
}

void bpp::BashppServer::sendResponse(const GenericResponseMessage& response) {
	std::string response_str = nlohmann::json(response).dump();
	_sendMessage(response_str);
	log("Sent response for request ID: ", response.id, ":", response_str);
}

void bpp::BashppServer::sendNotification(const GenericNotificationMessage& notification) {
	std::string notification_str = nlohmann::json(notification).dump();
	_sendMessage(notification_str);
	log("Sent notification for method: ", notification.method, ":", notification_str);
}

void bpp::BashppServer::processRequest(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;

	const auto* it = std::find_if(request_handlers.begin(), request_handlers.end(),
		[&request](const RequestHandlerEntry& entry) {
			return entry.method_name == request.method;
		}
	);

	if (it == request_handlers.end()) {
		log("No handler found for request method: ", request.method);
		response = invalidRequestHandler(request);
		sendResponse(response);
		return;
	}

	try {
		response = (this->*(it->handler))(request);
	} catch (const std::exception& e) {
		log("Error handling request: ", e.what());
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();
		response.error = err;
	}

	sendResponse(response);
}

void bpp::BashppServer::processNotification(const GenericNotificationMessage& notification) {
	const auto* it = std::find_if(notification_handlers.begin(), notification_handlers.end(),
		[&notification](const NotificationHandlerEntry& entry) {
			return entry.method_name == notification.method;
		}
	);

	if (it == notification_handlers.end()) {
		log("No handler found for notification method: ", notification.method);
		invalidNotificationHandler(notification);
		return;
	}

	try {
		(this->*(it->handler))(notification);
	} catch (const std::exception& e) {
		log("Error handling notification: ", e.what());
	}
}

void bpp::BashppServer::processMessage(const std::string& message) {
	GenericRequestMessage request;
	GenericNotificationMessage notification;

	nlohmann::json json_message;
	try {
		json_message = nlohmann::json::parse(message);
	} catch (const nlohmann::json::parse_error& e) {
		log("Error parsing JSON message: ", e.what());
		return;
	}

	// Request or notification?
	// Check if 'id' field is present
	// Requests are required to have IDs, notifications are required to not have IDs
	if (json_message.contains("id")) {
		request = json_message.get<GenericRequestMessage>();
		if (shutdown_requested) {
			// LSP spec says we should send an InvalidRequest error for any request received after shutdown is requested
			GenericResponseMessage response;
			response.id = request.id;
			response.error = ResponseError{
				static_cast<int>(ErrorCodes::InvalidRequest),
				"Server is shutting down and cannot process new requests.", nullptr};
			sendResponse(response);
			log("Received request after shutdown requested, sent error response.");
			return;
		}
		processRequest(request);
	} else {
		notification = json_message.get<GenericNotificationMessage>();
		processNotification(notification);
	}
}

GenericResponseMessage bpp::BashppServer::shutdown(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;
	response.result = nullptr; // No result for shutdown
	log("Received shutdown request, preparing to shut down the server.");
	shutdown_requested = true;
	return response;
}

void bpp::BashppServer::exit(const GenericNotificationMessage& /*notification*/) {
	log("Received exit notification, exiting the server.");
	cleanup();

	if (!shutdown_requested) {
		// LSP spec says that if we receive an exit notification without a shutdown request,
		// that we should exit with error code 1
		_exit(1);
	}
}

void bpp::BashppServer::publishDiagnostics(std::shared_ptr<bpp::bpp_program> program) {
	log("Publishing diagnostics for program rooted at: ", program->get_main_source_file());
	if (program == nullptr) {
		log("Cannot publish diagnostics for a null program.");
		return;
	}

	std::vector<std::string> source_files = program->get_source_files();
	for (const auto& file : source_files) {
		PublishDiagnosticsNotification notification;
		notification.params.uri = "file://" + file;
		std::vector<bpp::diagnostic> diags = program->get_diagnostics(file);
		std::vector<Diagnostic> lsp_diags;
		for (const auto& diag : diags) {
			Diagnostic lsp_diag;
			lsp_diag.range.start.line = diag.start_line;
			lsp_diag.range.start.character = diag.start_column;
			lsp_diag.range.end.line = diag.end_line;
			lsp_diag.range.end.character = diag.end_column;
			
			switch (diag.type) {
				case bpp::diagnostic_type::DIAGNOSTIC_ERROR:
					lsp_diag.severity = DiagnosticSeverity::Error;
					break;
				case bpp::diagnostic_type::DIAGNOSTIC_WARNING:
					lsp_diag.severity = DiagnosticSeverity::Warning;
					break;
				case bpp::diagnostic_type::DIAGNOSTIC_INFO:
					lsp_diag.severity = DiagnosticSeverity::Information;
					break;
				case bpp::diagnostic_type::DIAGNOSTIC_HINT:
					lsp_diag.severity = DiagnosticSeverity::Hint;
					break;
			}

			lsp_diag.message = diag.message;

			lsp_diags.push_back(lsp_diag);
		}

		notification.params.diagnostics = lsp_diags;
		log("Publishing ", lsp_diags.size(), " diagnostics for file: ", file);
		sendNotification(notification);
	}
}

void bpp::BashppServer::add_include_path(const std::string& path) {
	program_pool.add_include_path(path);
}

void bpp::BashppServer::set_suppress_warnings(bool suppress) {
	program_pool.set_suppress_warnings(suppress);
}
