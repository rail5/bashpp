/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BashppServer.h"

#include <unistd.h>

#include "generated/PublishDiagnosticsNotification.h"

std::mutex bpp::BashppServer::output_mutex;
std::mutex bpp::BashppServer::log_mutex;

bpp::BashppServer::BashppServer() {
	log("Bash++ Language Server initialized.");
	log("Using ", thread_pool.getThreadCount(), " threads for processing requests.");

	// Populate the default completion list
	default_completion_list.isIncomplete = false;

	CompletionItem i_class;
	i_class.label = "class";
	i_class.kind = CompletionItemKind::Keyword;
	i_class.detail = "Define a new class";
	default_completion_list.items.push_back(i_class);

	CompletionItem i_public;
	i_public.label = "public";
	i_public.kind = CompletionItemKind::Keyword;
	i_public.detail = "Public access specifier";
	default_completion_list.items.push_back(i_public);

	CompletionItem i_private;
	i_private.label = "private";
	i_private.kind = CompletionItemKind::Keyword;
	i_private.detail = "Private access specifier";
	default_completion_list.items.push_back(i_private);

	CompletionItem i_protected;
	i_protected.label = "protected";
	i_protected.kind = CompletionItemKind::Keyword;
	i_protected.detail = "Protected access specifier";
	default_completion_list.items.push_back(i_protected);

	CompletionItem i_method;
	i_method.label = "method";
	i_method.kind = CompletionItemKind::Keyword;
	i_method.detail = "Define a new method in a class";
	default_completion_list.items.push_back(i_method);

	CompletionItem i_constructor;
	i_constructor.label = "constructor";
	i_constructor.kind = CompletionItemKind::Keyword;
	i_constructor.detail = "Define a constructor for a class";
	default_completion_list.items.push_back(i_constructor);

	CompletionItem i_destructor;
	i_destructor.label = "destructor";
	i_destructor.kind = CompletionItemKind::Keyword;
	i_destructor.detail = "Define a destructor for a class";
	default_completion_list.items.push_back(i_destructor);

	CompletionItem i_virtual;
	i_virtual.label = "virtual";
	i_virtual.kind = CompletionItemKind::Keyword;
	i_virtual.detail = "Declare a method to be virtual";
	default_completion_list.items.push_back(i_virtual);

	CompletionItem i_this;
	i_this.label = "this";
	i_this.kind = CompletionItemKind::Keyword;
	i_this.detail = "Reference to the current object";
	default_completion_list.items.push_back(i_this);

	CompletionItem i_super;
	i_super.label = "super";
	i_super.kind = CompletionItemKind::Keyword;
	i_super.detail = "Reference to the current object's parent class";
	default_completion_list.items.push_back(i_super);

	CompletionItem i_include;
	i_include.label = "include";
	i_include.kind = CompletionItemKind::Keyword;
	i_include.detail = "Include a file";
	default_completion_list.items.push_back(i_include);
	
	CompletionItem i_include_once;
	i_include_once.label = "include_once";
	i_include_once.kind = CompletionItemKind::Keyword;
	i_include_once.detail = "Include a file only once";
	default_completion_list.items.push_back(i_include_once);

	CompletionItem i_dynamic_cast;
	i_dynamic_cast.label = "dynamic_cast";
	i_dynamic_cast.kind = CompletionItemKind::Keyword;
	i_dynamic_cast.detail = "Perform a dynamic cast";
	default_completion_list.items.push_back(i_dynamic_cast);

	CompletionItem i_new;
	i_new.label = "new";
	i_new.kind = CompletionItemKind::Keyword;
	i_new.detail = "Create a new object";
	default_completion_list.items.push_back(i_new);

	CompletionItem i_delete;
	i_delete.label = "delete";
	i_delete.kind = CompletionItemKind::Keyword;
	i_delete.detail = "Delete an object";
	default_completion_list.items.push_back(i_delete);

	CompletionItem i_nullptr;
	i_nullptr.label = "nullptr";
	i_nullptr.kind = CompletionItemKind::Keyword;
	i_nullptr.detail = "Null pointer constant";
	default_completion_list.items.push_back(i_nullptr);

}
bpp::BashppServer::~BashppServer() {}

void bpp::BashppServer::setInputStream(std::shared_ptr<std::istream> stream) {
	input_stream = stream;
}

void bpp::BashppServer::setOutputStream(std::shared_ptr<std::ostream> stream) {
	output_stream = stream;
}

void bpp::BashppServer::setSocketPath(const std::string& path) {
	socket_path = path;
}

void bpp::BashppServer::setLogFile(const std::string& path) {
	log_file.open(path, std::ios::app);
	if (!log_file.is_open()) {
		throw std::runtime_error("Failed to open log file: " + path);
	}
}

void bpp::BashppServer::setTargetBashVersion(const BashVersion& version) {
	program_pool.set_target_bash_version(version);
}

void bpp::BashppServer::cleanup() {
	if (output_stream) {
		output_stream->flush();
	}

	if (socket_path.has_value()) {
		// Unlink the Unix socket file if it exists
		unlink(socket_path.value().c_str());
	}

	thread_pool.cleanup();
	log("Bash++ Language Server cleaned up and exiting.");
	log_file.close();
}

std::string bpp::BashppServer::readHeaderLine(std::streambuf* buffer) {
	std::string header;
	char c;
	while (true) {
		const std::streamsize n = buffer->sgetn(&c, 1);
		if (n != 1) {
			throw;
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

	std::streambuf* buffer = input_stream->rdbuf();
	while (true) {
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
		if (header.find("Content-Length: ") == 0) {
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
		thread_pool.enqueue([this, message]() {
			try {
				this->processMessage(message);
			} catch (const std::exception& e) {
				log("Error processing message: ", e.what());
			}
		});
	}
}

const GenericResponseMessage bpp::BashppServer::invalidRequestHandler(const GenericRequestMessage& request) {
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
	std::function<GenericResponseMessage(const GenericRequestMessage&)> request_handler = invalidRequestHandler;
	auto it = request_handlers.find(frozen::string(request.method.c_str()));
	if (it != request_handlers.end()) {
		request_handler = std::bind(it->second, this, std::placeholders::_1); // Bind the method to the current instance
	} else {
		log("No handler found for request method: ", request.method);
	}

	try {
		response = request_handler(request);
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
	std::function<void(const GenericNotificationMessage&)> notification_handler = invalidNotificationHandler;
	auto it = notification_handlers.find(frozen::string(notification.method.c_str()));
	if (it != notification_handlers.end()) {
		notification_handler = std::bind(it->second, this, std::placeholders::_1); // Bind the method to the current instance
	} else {
		log("No handler found for notification method: ", notification.method);
	}

	try {
		notification_handler(notification);
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
		processRequest(request);
	} else {
		notification = json_message.get<GenericNotificationMessage>();
		processNotification(notification);
	}

	if (request.method == "shutdown") {
		log("Received shutdown request, cleaning up and exiting.");
		cleanup();
		exit(0);
	}
}

GenericResponseMessage bpp::BashppServer::shutdown(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;
	response.result = nullptr; // No result for shutdown
	log("Received shutdown request, preparing to shut down the server.");
	return response;
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
