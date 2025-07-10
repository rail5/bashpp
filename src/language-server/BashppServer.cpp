/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BashppServer.h"

#include "static/Message.h"
#include "generated/InitializeRequest.h"
#include "generated/InitializeResult.h"
#include "generated/ErrorCodes.h"
#include "generated/DefinitionRequest.h"
#include "generated/CompletionRequest.h"
#include "generated/HoverRequest.h"
#include "generated/DocumentSymbolRequest.h"
#include "generated/RenameRequest.h"

std::mutex BashppServer::cout_mutex;

BashppServer::BashppServer() {}
BashppServer::~BashppServer() {}

void BashppServer::processMessage(const std::string& message) {
	GenericRequestMessage request;
	std::function<nlohmann::json(const nlohmann::json&)> handler = nullptr; // Function pointer to be set to the appropriate handler
	try {
		request = nlohmann::json::parse(message).get<GenericRequestMessage>();

		auto it = handlers.find(request.method);
		if (it == handlers.end()) {
			log_file << "No handler found for method: " << request.method << std::endl;
			return;
		}
		handler = it->second; // Set the handler to the appropriate function
	} catch (const std::exception& e) {
		std::cerr << "Error parsing message: " << e.what() << std::endl;
		return;
	}

	if (!handler) {
		log_file << "No handler found for method: " << request.method << std::endl;
		return;
	}

	// Call the handler and get the result
	GenericResponseMessage response;
	try {
		response = handler(request);
	} catch (const std::exception& e) {
		log_file << "Error handling request: " << e.what() << std::endl;
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();
		response.error = err;
	}

	if (response.error.code != 0) {
		log_file << "Error in response: " << response.error.message << std::endl;
	}

	std::string response_str = nlohmann::json(response).dump();
	std::string header = "Content-Length: " + std::to_string(response_str.size()) + "\r\n\r\n";
	std::lock_guard<std::mutex> lock(cout_mutex); // Ensure thread-safe output
	std::cout << header << response_str << std::flush;
	log_file << "Sent response for method: " << request.method << std::endl
		<< response_str << std::endl;

	if (request.method == "shutdown") {
		log_file << "Shutting down server" << std::endl;
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

/*
json BashppServer::handleInitialize(const json& params) {
	json result;
	result = {"result", {
		{"capabilities", {
			{"textDocumentSync", 1}, // Full sync mode
			{"hoverProvider", true},
			{"completionProvider", {
					{"resolveProvider", false},
					{"triggerCharacters", {".", "@"}}
				}
			},
			{"definitionProvider", true},
			{"renameProvider", true},
			{"documentSymbolProvider", true},
			{"workspaceSymbolProvider", true}
		}}
	}};
	return result;
}*/

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

	return response.toGeneric();
}

GenericResponseMessage BashppServer::handleGotoDefinition(const GenericRequestMessage& request) {
	// Placeholder for actual implementation
	DefinitionRequestResponse response;
	response.id = request.id;
	try {
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
	} catch (const std::exception& e) {
		log_file << "Error handling GotoDefinition: " << e.what() << std::endl;
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();

		response.error = err;
	}

	return response;
}

GenericResponseMessage BashppServer::handleCompletion(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	CompletionRequestResponse response;
	response.id = request.id;
	try {
		CompletionRequest completion_request = request.toSpecific<CompletionParams>();

		// TBD
		CompletionItem item;
		item.label = "example";
		item.kind = CompletionItemKind::Text;
		item.detail = "Example completion item";
		item.documentation = "This is an example completion item.";

		CompletionList completion_list;
		completion_list.isIncomplete = false;

		completion_list.items.push_back(item);

		response.result = completion_list;

	} catch (const std::exception& e) {
		log_file << "Error handling Completion: " << e.what() << std::endl;
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();

		response.error = err;
	}


	return response;
}

GenericResponseMessage BashppServer::handleHover(const GenericRequestMessage& request) {
	// Placeholder for actual implementation

	HoverRequestResponse response;
	response.id = request.id;

	try {
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

	} catch (const std::exception& e) {
		log_file << "Error handling Hover: " << e.what() << std::endl;
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();

		response.error = err;
	}

	return response;
}

GenericResponseMessage BashppServer::handleDocumentSymbol(const GenericRequestMessage& request) {
	// Placeholder for actual implementation
	/*json result = {"result", {
		{"symbols", {
			{
				{"name", "ExampleClass"},
				{"kind", 5}, // Class
				{"location", {
					{"uri", "file:///path/to/file.bpp"},
					{"range", {
						{"start", {"line", 0, "character", 0}},
						{"end", {"line", 0, "character", 10}}
					}}
				}},
				{"children", {}}
			}
		}}
	}};
	return result;*/

	DocumentSymbolRequestResponse response;
	response.id = request.id;
	try {
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
	} catch (const std::exception& e) {
		log_file << "Error handling DocumentSymbol: " << e.what() << std::endl;
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();

		response.error = err;
	}

	return response;
}

GenericResponseMessage BashppServer::handleDidOpen(const GenericRequestMessage& request) {
	// Placeholder for actual implementation
	/*json result = {"result", {
		{"uri", params["textDocument"]["uri"]},
		{"languageId", "bashpp"},
		{"version", 1},
		{"text", params["textDocument"]["text"]}
	}};
	return result;*/

	// didOpen is a notification, not a request, so we don't return a response
	GenericResponseMessage response;
	return response;
}

GenericResponseMessage BashppServer::handleDidChange(const GenericRequestMessage& request) {
	// Placeholder for actual implementation
	/*json result = {"result", {
		{"uri", params["textDocument"]["uri"]},
		{"version", params["textDocument"]["version"]},
		{"contentChanges", params["contentChanges"]}
	}};
	return result;*/

	// didChange is a notification, not a request, so we don't return a response
	GenericResponseMessage response;
	return response;
}

GenericResponseMessage BashppServer::handleRename(const GenericRequestMessage& request) {
	// Placeholder for actual implementation
	/*json result = {"result", {
		{"changes", {
			{ params["textDocument"]["uri"], json::array({
				{
					{"range", {
						{"start", {{"line", params["position"]["line"]}, {"character", params["position"]["character"]}}},
						{"end",   {{"line", params["position"]["line"]}, {"character", static_cast<int>(params["position"]["character"]) + static_cast<int>(params["newName"].size())}}}
					}},
					{"newText", params["newName"]}
				}
			})}
		}}
	}};
	return result;*/

	RenameRequestResponse response;
	response.id = request.id;

	try {
		RenameRequest rename_request = request.toSpecific<RenameParams>();
		std::string uri = rename_request.params.textDocument.uri;
		Position position = rename_request.params.position;
		std::string new_name = rename_request.params.newName;

		WorkspaceEdit edit;
		TextEdit text_edit;
		text_edit.range.start.line = position.line;
		text_edit.range.start.character = position.character;
		text_edit.range.end.line = position.line;
		text_edit.range.end.character = position.character + static_cast<int>(new_name.size());
		text_edit.newText = new_name;

		if (!edit.changes.has_value()) {
			edit.changes = std::unordered_map<std::string, std::vector<TextEdit>>{};
		}
		edit.changes->operator[](uri).push_back(text_edit);

		response.result = edit;

	} catch (const std::exception& e) {
		log_file << "Error handling Rename: " << e.what() << std::endl;
		ResponseError err;
		err.code = static_cast<int>(ErrorCodes::InternalError);
		err.message = "Internal error";
		err.data = e.what();

		response.error = err;
	}

	return response;
}
