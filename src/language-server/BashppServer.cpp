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

BashppServer::BashppServer() {}
BashppServer::~BashppServer() {}

void BashppServer::setOutputStream(std::shared_ptr<std::ostream> stream) {
	output_stream = stream;
}
#include <iostream>
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
}

const GenericResponseMessage BashppServer::invalidRequestHandler(const GenericRequestMessage& request) {
	throw std::runtime_error("Request not understood: " + request.method);
}

const void BashppServer::invalidNotificationHandler(const GenericNotificationMessage& request) {
	throw std::runtime_error("Notification not understood: " + request.method);
}

void BashppServer::sendResponse(const GenericResponseMessage& response) {
	std::string response_str = nlohmann::json(response).dump();
	std::string header = "Content-Length: " + std::to_string(response_str.size()) + "\r\n\r\n";
	std::lock_guard<std::mutex> lock(output_mutex);
	*output_stream << header << response_str << std::flush;
	log_file << "Sent response for request: " << std::endl
		<< response_str << std::endl;
}

void BashppServer::sendNotification(const GenericNotificationMessage& notification) {
	std::string notification_str = nlohmann::json(notification).dump();
	std::string header = "Content-Length: " + std::to_string(notification_str.size()) + "\r\n\r\n";
	std::lock_guard<std::mutex> lock(output_mutex);
	*output_stream << header << notification_str << std::flush;
	log_file << "Sent notification for method: " << notification.method << std::endl
		<< notification_str << std::endl;
}

void BashppServer::processRequest(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;
	std::function<GenericResponseMessage(const GenericRequestMessage&)> request_handler = invalidRequestHandler;
	auto it = request_handlers.find(request.method);
	if (it != request_handlers.end()) {
		request_handler = it->second; // Set the handler to the appropriate function
	} else {
		log_file << "No handler found for request method: " << request.method << std::endl;
	}

	try {
		response = request_handler(request);
	} catch (const std::exception& e) {
		log_file << "Error handling request: " << e.what() << std::endl;
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
		log_file << "No handler found for notification method: " << notification.method << std::endl;
	}

	try {
		notification_handler(notification);
	} catch (const std::exception& e) {
		log_file << "Error handling notification: " << e.what() << std::endl;
	}
}

void BashppServer::processMessage(const std::string& message) {
	GenericRequestMessage request;
	GenericNotificationMessage notification;

	nlohmann::json json_message;
	try {
		json_message = nlohmann::json::parse(message);
	} catch (const nlohmann::json::parse_error& e) {
		log_file << "JSON parse error: " << e.what() << std::endl;
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
		log_file << "Shutting down server" << std::endl;
		cleanup();
		exit(0);
	}
}

GenericResponseMessage BashppServer::shutdown(const GenericRequestMessage& request) {
	GenericResponseMessage response;
	response.id = request.id;
	response.result = nullptr; // No result for shutdown
	log_file << "Received shutdown request" << std::endl;
	return response;
}

GenericResponseMessage BashppServer::handleInitialize(const GenericRequestMessage& request) {
	InitializeRequest initialize_request = request.toSpecific<InitializeParams>();
	InitializeRequestResponse response;
	response.id = request.id;

	InitializeResult result;
	result.capabilities.textDocumentSync = static_cast<TextDocumentSyncKind>(1); // Full sync mode
	result.capabilities.hoverProvider = true;
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

	log_file << "Received GotoDefinition request for URI: " << uri << ", Position: (" 
		<< position.line << ", " << position.character << ")" << std::endl;

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

	log_file << "Received Hover request for URI: " << uri << ", Position: (" 
		<< position.line << ", " << position.character << ")" << std::endl;

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
