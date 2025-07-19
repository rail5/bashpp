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
#include "generated/DidCloseTextDocumentNotification.h"
#include "generated/ShowMessageNotification.h"
#include "generated/PublishDiagnosticsNotification.h"

std::mutex BashppServer::output_mutex;
std::mutex BashppServer::log_mutex;

BashppServer::BashppServer() {
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

	CompletionRequestResponse response;
	response.id = request.id;
	CompletionRequest completion_request = request.toSpecific<CompletionParams>();

	std::string uri = completion_request.params.textDocument.uri;
	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to provide completions for non-local file: ", uri);
		response.result = nullptr;
		return response;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}
	
	// Which character triggered the request?
	char trigger_character = '@';
	if (completion_request.params.context.has_value() && completion_request.params.context->triggerCharacter.has_value()) {
		trigger_character = completion_request.params.context->triggerCharacter.value()[0];
	}

	CompletionList completion_list;
	completion_list.isIncomplete = false; // Assume completions are complete for now
	
	// If it was '@', suggest class names, object names, and standard operators like include or dynamic_cast
	// If it was '.', suggest method names and data members of the current object
	// Also, '.' requires for us to wait until previous unsaved changes have been processed (done by default)
	switch (trigger_character) {
		case '@':
		default:
		{
			log("Received Completion request for URI: ", uri, " with trigger character: '@'");

			completion_list = default_completion_list; // Start with the default completion list

			std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri, true); // Jump the queue and get the program immediately
			if (program == nullptr) {
				log("Program not found for URI: ", uri);
				response.result = nullptr;
				return response;
			}

			Position position = completion_request.params.position;
			
			// Which entity is active at the given position?
			std::shared_ptr<bpp::bpp_entity> active_entity = program->get_active_entity(uri, position.line, position.character);
			if (active_entity == nullptr) {
				log("BUG: No active entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri, " - returning default completions.");
			} else {
				auto objects = active_entity->get_objects();
				auto classes = active_entity->get_classes();

				for (const auto& obj : objects) {
					CompletionItem item;
					item.label = obj.first;
					item.kind = CompletionItemKind::Variable;
					item.detail = "@" + obj.second->get_class()->get_name() + " " + obj.first; // As in: @ClassName objectName
					completion_list.items.push_back(item);
				}

				for (const auto& cls : classes) {
					CompletionItem item;
					item.label = cls.first;
					item.kind = CompletionItemKind::Class;
					item.detail = "@class " + cls.second->get_name(); // As in: @class ClassName
					completion_list.items.push_back(item);
				}
			}
		}
		break;
		case '.':
		{
			log("Received Completion request for URI: ", uri, " with trigger character: '.'");
			// Wait for stored_changes_content_updating to be false before proceeding
			// Just to make sure we're suggesting completions based on the latest content
			// When the user types '.', the client will send a didChange notification,
			// and then immediately after, it'll send a completion request.
			// We need to ensure that our internally-stored version of the file content
			// is up-to-date before we resolve the reference and provide completions.
			while (stored_changes_content_updating) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for 10 milliseconds
			}
			std::lock_guard<std::mutex> lock(unsaved_changes_mutex);

			std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri, true); // Jump the queue and get the program immediately
			if (program == nullptr) {
				log("Program not found for URI: ", uri);
				response.result = nullptr;
				return response;
			}

			Position position = completion_request.params.position;

			// Resolve the referenced entity before the dot
			bpp::entity_reference referenced_entity;

			if (unsaved_changes.find(uri) != unsaved_changes.end()) {
				referenced_entity = bpp::resolve_reference_at(uri, position.line, position.character, program, unsaved_changes[uri]);
			} else {
				referenced_entity = bpp::resolve_reference_at(uri, position.line, position.character, program);
			}

			if (referenced_entity.entity == nullptr) {
				log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri, " - returning default completions.");
				response.result = nullptr;
				return response;
			}

			// Ensure that the referenced entity is a non-primitive object
			std::shared_ptr<bpp::bpp_object> obj = std::dynamic_pointer_cast<bpp::bpp_object>(referenced_entity.entity);
			if (obj == nullptr || obj->get_class() == program->get_primitive_class()) {
				log("Referenced entity is not a valid object or is a primitive type at position: (", position.line, ", ", position.character, ") in URI: ", uri, " - returning default completions.");
				response.result = nullptr;
				return response;
			}

			// Otherwise, let's populate the completion list with methods and data members of the object's class
			for (const auto& method : obj->get_class()->get_methods()) {
				if (method->get_name().find("__") != std::string::npos) {
					continue; // Skip system methods
				}

				CompletionItem item;
				item.label = method->get_name();
				item.kind = CompletionItemKind::Method;

				std::string detail = "";

				if (method->is_virtual()) {
					detail += "@virtual ";
				}
				
				switch (method->get_scope()) {
					case bpp::bpp_scope::SCOPE_PUBLIC:
						detail += "@public ";
						break;
					case bpp::bpp_scope::SCOPE_PRIVATE:
						detail += "@private ";
						break;
					case bpp::bpp_scope::SCOPE_PROTECTED:
						detail += "@protected ";
						break;
					default:
						detail += "@private ";
						break;
				}

				detail += "@method " + method->get_name();

				for (const auto& param : method->get_parameters()) {
					detail += " [";
					if (param->get_type() == program->get_primitive_class()) {
						detail += "$" + param->get_name();
					} else {
						detail += "@" + param->get_type()->get_name() + "* " + param->get_name();
					}
					detail += "]";
				}

				item.detail = detail;
				// Full example: @virtual @public @method methodName [@ClassName* param1] [$primitive_param2]

				completion_list.items.push_back(item);
			}

			for (const auto& data_member : obj->get_class()->get_datamembers()) {
				CompletionItem item;
				item.label = data_member->get_name();
				item.kind = CompletionItemKind::Field;

				std::string detail = "@" + obj->get_name() + "." + data_member->get_name();
				detail += " (";

				if (data_member->get_class() == program->get_primitive_class()) {
					detail += "primitive";
					if (data_member->is_array()) {
						detail += " array";
					}
				} else {
					detail += "@" + data_member->get_class()->get_name();

					if (data_member->is_pointer()) {
						detail += "*";
					}
				}
				detail += ")";

				item.detail = detail;
				// Full example: @objectName.dataMemberName (primitive)
				// Or: @objectName.dataMemberName (primitive array)
				// Or: @objectName.dataMemberName (@ClassName)
				// Or: @objectName.dataMemberName (@ClassName*)

				completion_list.items.push_back(item);
			}
		}
		break;
	}

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
	program_pool.open_file(uri); // Mark the file as open
	return;
}

void BashppServer::handleDidChange(const GenericNotificationMessage& request) {
	stored_changes_content_updating = true; // Set the flag to indicate that changes are pending
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

	// Update the "unsaved changes" stored
	{
		std::lock_guard<std::mutex> changes_lock(unsaved_changes_mutex);
		if (did_change_notification.params.contentChanges.size() == 1
			&& std::holds_alternative<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0))) {
			unsaved_changes[uri] = std::get<TextDocumentContentChangeWholeDocument>(did_change_notification.params.contentChanges.at(0)).text;
			stored_changes_content_updating = false;
		} else {
			log("Ignoring DidChange notification for URI: ", uri, " as it either has multiple content changes or is not a whole document change.");
			return;
		}
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

		{
			std::lock_guard<std::mutex> lock(unsaved_changes_mutex);
			auto it = unsaved_changes.find(uri);
			if (it != unsaved_changes.end()) {
				log("Forgetting unsaved changes for URI: ", uri);
				unsaved_changes.erase(it); // Remove unsaved changes for this URI
			}
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

void BashppServer::handleDidClose(const GenericNotificationMessage& request) {
	DidCloseTextDocumentNotification did_close_notification = request.toSpecific<DidCloseTextDocumentParams>();
	std::string uri = did_close_notification.params.textDocument.uri;

	log("Received DidClose notification for URI: ", uri);

	// Ensure the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to close non-local file: ", uri);
		return;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	// If we've stored unsaved changes for this URI, we can remove them
	{
		std::lock_guard<std::mutex> lock(unsaved_changes_mutex);
		auto it = unsaved_changes.find(uri);
		if (it != unsaved_changes.end()) {
			log("Forgetting unsaved changes for URI: ", uri);
			unsaved_changes.erase(it);
		}
	}

	program_pool.close_file(uri); // Mark the file as closed
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
