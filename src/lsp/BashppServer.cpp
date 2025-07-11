/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BashppServer.h"

#include <unistd.h>

#include "static/Message.h"
#include "generated/InitializeRequest.h"
#include "generated/InitializeResult.h"
#include "generated/ErrorCodes.h"
#include "generated/DefinitionRequest.h"
#include "generated/CompletionRequest.h"
#include "generated/HoverRequest.h"
#include "generated/DocumentSymbolRequest.h"
#include "generated/RenameRequest.h"
#include "generated/DidOpenTextDocumentNotification.h"
#include "generated/ShowMessageNotification.h"

std::mutex BashppServer::output_mutex;
std::mutex BashppServer::log_mutex;

BashppServer::BashppServer() {
	log("Bash++ Language Server initialized.");
	log("Using ", thread_pool.getThreadCount(), " threads for processing requests.");
}
BashppServer::~BashppServer() {}

void BashppServer::setInputStream(std::shared_ptr<std::istream> stream) {
	input_stream = stream;
}

void BashppServer::setOutputStream(std::shared_ptr<std::ostream> stream) {
	output_stream = stream;
}

void BashppServer::setSocketPath(const std::string& path) {
	socket_path = path;
}

void BashppServer::cleanup() {
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

void BashppServer::mainLoop() {
	if (!input_stream || !output_stream) {
		throw std::runtime_error("Input or output stream not set.");
	}

	std::streambuf* buffer = input_stream->rdbuf();
	while (true) {
		// Read headers
		std::string header;
		char c;
		while (buffer->sgetn(&c, 1) == 1 && c != '\n') {
			if (c == '\r') continue;
			header += c;
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
		if (buffer->sgetn(content.data(), content_length) != content_length) {
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

const GenericResponseMessage BashppServer::invalidRequestHandler(const GenericRequestMessage& request) {
	throw std::runtime_error("Request not understood: " + request.method);
}

const void BashppServer::invalidNotificationHandler(const GenericNotificationMessage& request) {
	throw std::runtime_error("Notification not understood: " + request.method);
}

void BashppServer::_sendMessage(const std::string& message) {
	std::lock_guard<std::mutex> lock(output_mutex);
	std::string header = "Content-Length: " + std::to_string(message.size()) + "\r\n\r\n";
	*output_stream << header << message << std::flush;
}

void BashppServer::sendResponse(const GenericResponseMessage& response) {
	std::string response_str = nlohmann::json(response).dump();
	_sendMessage(response_str);
	log("Sent response for request ID: ", response.id, ":", response_str);
}

void BashppServer::sendNotification(const GenericNotificationMessage& notification) {
	std::string notification_str = nlohmann::json(notification).dump();
	_sendMessage(notification_str);
	log("Sent notification for method: ", notification.method, ":", notification_str);
}

void BashppServer::processRequest(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;
	std::function<GenericResponseMessage(const GenericRequestMessage&)> request_handler = invalidRequestHandler;
	auto it = request_handlers.find(request.method);
	if (it != request_handlers.end()) {
		request_handler = it->second; // Set the handler to the appropriate function
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

void BashppServer::processNotification(const GenericNotificationMessage& notification) {
	std::function<void(const GenericNotificationMessage&)> notification_handler = invalidNotificationHandler;
	auto it = notification_handlers.find(notification.method);
	if (it != notification_handlers.end()) {
		notification_handler = it->second; // Set the handler to the appropriate function
	} else {
		log("No handler found for notification method: ", notification.method);
	}

	try {
		notification_handler(notification);
	} catch (const std::exception& e) {
		log("Error handling notification: ", e.what());
	}
}

void BashppServer::processMessage(const std::string& message) {
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

GenericResponseMessage BashppServer::shutdown(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;
	response.result = nullptr; // No result for shutdown
	log("Received shutdown request, preparing to shut down the server.");
	return response;
}

GenericResponseMessage BashppServer::handleInitialize(const GenericRequestMessage& request) {
	InitializeRequest initialize_request = request.toSpecific<InitializeParams>();
	InitializeRequestResponse response;
	response.id = request.id;

	InitializeResult result;
	result.capabilities.textDocumentSync = static_cast<TextDocumentSyncKind>(1); // Full sync mode
	result.capabilities.hoverProvider = true;

	CompletionOptions completionProvider;
	result.capabilities.completionProvider = completionProvider;
	result.capabilities.completionProvider->resolveProvider = false;
	result.capabilities.completionProvider->triggerCharacters = {".", "@"};

	result.capabilities.definitionProvider = true;
	result.capabilities.renameProvider = true;
	result.capabilities.documentSymbolProvider = true;
	result.capabilities.workspaceSymbolProvider = true;

	response.result = result;

	return response;
}

GenericResponseMessage BashppServer::handleGotoDefinition(const GenericRequestMessage& request) {
	// Placeholder for actual implementation
	DefinitionRequestResponse response;
	response.id = request.id;
	DefinitionRequest definition_request = request.toSpecific<DefinitionParams>();
	std::string uri = definition_request.params.textDocument.uri;
	Position position = definition_request.params.position;

	log("Received Goto Definition request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	Location location;
	location.uri = "file:///usr/lib/bpp/stdlib/SharedStack";
	location.range.start.line = 72;
	location.range.start.character = 1;
	location.range.end.line = 72;
	location.range.end.character = 8;

	Definition definition = location;

	response.result = definition;

	return response;
}

GenericResponseMessage BashppServer::handleCompletion(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	CompletionRequestResponse response;
	response.id = request.id;
	CompletionRequest completion_request = request.toSpecific<CompletionParams>();

	CompletionItem item;
	item.label = "example";
	item.kind = CompletionItemKind::Text;
	item.detail = "Example completion item";
	item.documentation = "This is an example completion item.";

	CompletionList completion_list;
	completion_list.isIncomplete = false;

	completion_list.items.push_back(item);

	response.result = completion_list;

	return response;
}

GenericResponseMessage BashppServer::handleHover(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	HoverRequestResponse response;
	response.id = request.id;
	HoverRequest hover_request = request.toSpecific<HoverParams>();
	std::string uri = hover_request.params.textDocument.uri;
	Position position = hover_request.params.position;

	log("Received Hover request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	Hover hover;
	MarkupContent hoverContent;
	hoverContent.kind = MarkupKind::Markdown;
	hoverContent.value = "This is a hover message.";
	hover.contents = hoverContent;

	response.result = hover;

	return response;
}

GenericResponseMessage BashppServer::handleDocumentSymbol(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	DocumentSymbolRequestResponse response;
	response.id = request.id;
	DocumentSymbolRequest document_symbol_request = request.toSpecific<DocumentSymbolParams>();

	DocumentSymbol symbol;
	symbol.name = "ExampleClass";
	symbol.kind = SymbolKind::Class;
	symbol.range.start.line = 0;
	symbol.range.start.character = 0;
	symbol.range.end.line = 0;
	symbol.range.end.character = 10;
	symbol.selectionRange.start.line = 0;
	symbol.selectionRange.start.character = 0;
	symbol.selectionRange.end.line = 0;
	symbol.selectionRange.end.character = 10;

	response.result = std::vector<DocumentSymbol>{symbol};

	return response;
}

GenericResponseMessage BashppServer::handleRename(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	RenameRequestResponse response;
	response.id = request.id;
	RenameRequest rename_request = request.toSpecific<RenameParams>();
	std::string uri = rename_request.params.textDocument.uri;
	Position position = rename_request.params.position;
	std::string new_name = rename_request.params.newName;

	WorkspaceEdit edit;
	TextEdit text_edit;
	text_edit.range.start.line = position.line;
	text_edit.range.start.character = position.character;
	text_edit.range.end.line = position.line;
	text_edit.range.end.character = position.character + static_cast<uint32_t>(new_name.size());
	text_edit.newText = new_name;

	if (!edit.changes.has_value()) {
		edit.changes = std::unordered_map<std::string, std::vector<TextEdit>>{};
	}
	edit.changes->operator[](uri).push_back(text_edit);

	response.result = edit;

	return response;
}

void BashppServer::handleDidOpen(const GenericNotificationMessage& request) {
	// Placeholder for actual implementation
	/*json result = {"result", {
		{"uri", params["textDocument"]["uri"]},
		{"languageId", "bashpp"},
		{"version", 1},
		{"text", params["textDocument"]["text"]}
	}};
	return result;*/

	// didOpen is a notification, not a request, so we don't return a response

	// For fun, however, let's send a notification back just to prove we can
	DidOpenTextDocumentNotification did_open_notification = request.toSpecific<DidOpenTextDocumentParams>();
	ShowMessageNotification notification;
	notification.params.type = MessageType::Info;
	notification.params.message = "Opened file: " + did_open_notification.params.textDocument.uri;
	sendNotification(notification);
	return;
}

void BashppServer::handleDidChange(const GenericNotificationMessage& request) {
	// Placeholder for actual implementation
	/*json result = {"result", {
		{"uri", params["textDocument"]["uri"]},
		{"version", params["textDocument"]["version"]},
		{"contentChanges", params["contentChanges"]}
	}};
	return result;*/

	// didChange is a notification, not a request, so we don't return a response
	return;
}
