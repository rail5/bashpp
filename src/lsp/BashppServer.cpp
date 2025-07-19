/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BashppServer.h"

#include <unistd.h>

#include "../bpp_include/bpp_codegen.h"

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
#include "generated/DidChangeTextDocumentNotification.h"
#include "generated/DidChangeWatchedFilesNotification.h"
#include "generated/ShowMessageNotification.h"
#include "generated/PublishDiagnosticsNotification.h"

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

void BashppServer::setLogFile(const std::string& path) {
	log_file.open(path, std::ios::app);
	if (!log_file.is_open()) {
		throw std::runtime_error("Failed to open log file: " + path);
	}
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
	DefinitionRequest definition_request = request.toSpecific<DefinitionParams>();
	DefinitionRequestResponse response;
	response.id = request.id;

	std::string uri = definition_request.params.textDocument.uri;
	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to go to definition for non-local file: ", uri);
		response.error.code = static_cast<int>(ErrorCodes::InvalidParams);
		response.error.message = "Invalid URI: " + uri;
		return response;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	Position position = definition_request.params.position;
	log("Received Goto Definition request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri);

	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.error.code = static_cast<int>(ErrorCodes::InternalError);
		response.error.message = "Program not found for URI: " + uri;
		return response;
	}

	bpp::entity_reference referenced_entity = bpp::resolve_reference_at(uri, position.line, position.character, program);

	if (referenced_entity.entity == nullptr) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	bpp::SymbolPosition definition_location = referenced_entity.entity->get_initial_definition();

	Location location;
	location.uri = "file://" + definition_location.file;
	location.range.start.line = definition_location.line;
	location.range.start.character = definition_location.column;
	location.range.end.line = definition_location.line;
	location.range.end.character = definition_location.column + referenced_entity.entity->get_name().size();

	response.result = std::vector<Location>{location};
	log("Found definition for entity: ", referenced_entity.entity->get_name(), " at URI: ", uri, ", Range: (", location.range.start.line, ", ", location.range.start.character, ") to (", location.range.end.line, ", ", location.range.end.character, ")");
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
	DidOpenTextDocumentNotification did_open_notification = request.toSpecific<DidOpenTextDocumentParams>();
	
	log("Received DidOpen notification for URI: ", did_open_notification.params.textDocument.uri);

	// Ensure the URI starts with "file://"
	// We will reject any URIs that do not point to a file
	std::string uri = did_open_notification.params.textDocument.uri;
	if (uri.find("file://") != 0) {
		log("Ignoring request to parse non-local file: ", uri);
		return;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri);
	log("Finished parsing: ", uri);
	if (program == nullptr) {
		log("Failed to parse program: ", uri);
		return;
	}
	publishDiagnostics(program);
	return;
}

void BashppServer::handleDidChange(const GenericNotificationMessage& request) {
	DidChangeTextDocumentNotification did_change_notification = request.toSpecific<DidChangeTextDocumentParams>();
	std::string uri = did_change_notification.params.textDocument.uri;

	log("Received DidChange notification for URI: ", uri);

	// Ensure the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to re-parse non-local file: ", uri);
		return;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	// Debounce didChange processing
	uint64_t now = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()); // Record current timestamp
	{
		std::lock_guard<std::mutex> lock(debounce_mutex);
		if (!debounce_timestamps.count(uri)) {
			debounce_timestamps[uri] = std::make_shared<std::atomic<uint64_t>>(0);
		}
		debounce_timestamps[uri]->store(now); // Store the previously-recorded timestamp in the map
	}

	std::thread([this, uri, now, did_change_notification]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1 second debounce
		bool should_parse = false;
		{
			std::lock_guard<std::mutex> lock(debounce_mutex);
			if (debounce_timestamps[uri]->load() == now) {
				should_parse = true; // Only re-parse if this thread's "now" is the last one that was stored in the map
				// Earlier threads had their timestamps overwritten and will not continue
			}
		}

		if (should_parse) {
			log("Re-parsing program for URI: ", uri);

			// Get the unsaved content from the notification
			// If there are no content changes, we will re-parse the file as it is on disk
			// If there are content changes, we will re-parse the file
			// with the unsaved content provided in the notification

			// Per the LSP spec, contentChanges is an array of either:
			// 1. TextDocumentContentChangeWholeDocument
			// 2. TextDocumentContentChangePartial

			// At the moment, we only support whole document changes
			// The logic to support partial changes probably won't be too complicated altogether,
			// But regardless, that's for later.

			// We'll also only handle the case where there is exactly one change
			// If there are multiple changes, we will ignore them for now

			// TODO(@rail5): Handling partial changes and handling multiple changes (possibly of different types) **must** be implemented in the future

			std::string unsaved_content;
			if (did_change_notification.params.contentChanges.size() == 1
				&& std::holds_alternative<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0))) {
				unsaved_content = std::get<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0)).text;
			} else {
				log("Ignoring DidChange notification for URI: ", uri, " as it either has multiple content changes or is not a whole document change.");
				return;
			}
			
			if (!unsaved_content.empty()) {
				std::shared_ptr<bpp::bpp_program> program = program_pool.re_parse_program(uri, {uri, unsaved_content});
				if (program != nullptr) {
					publishDiagnostics(program);
				} else {
					log("Failed to re-parse program: ", uri);
				}
			}
		}
	}).detach();
}

void BashppServer::handleDidChangeWatchedFiles(const GenericNotificationMessage& request) {
	DidChangeWatchedFilesNotification did_change_notification = request.toSpecific<DidChangeWatchedFilesParams>();
	std::vector<FileEvent> changes = did_change_notification.params.changes;
	if (changes.empty()) {
		log("Received empty DidChangeWatchedFiles notification, ignoring.");
		return;
	}
	log("Received DidChangeWatchedFiles notification for ", changes.size(), " files.");
	
	for (const auto& change : changes) {
		log("File change detected: ", change.uri);
		// Ensure the URI starts with "file://"
		std::string uri = change.uri;
		if (uri.find("file://") != 0) {
			log("Ignoring request to re-parse non-local file: ", uri);
			return;
		} else {
			// Strip the "file://" prefix
			uri = uri.substr(7);
		}

		log("Re-parsing program for URI: ", uri);
		std::shared_ptr<bpp::bpp_program> program = program_pool.re_parse_program(uri);
		if (program != nullptr) {
			publishDiagnostics(program);
		} else {
			log("Failed to re-parse program: ", uri);
		}
	}
}

void BashppServer::publishDiagnostics(std::shared_ptr<bpp::bpp_program> program) {
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
