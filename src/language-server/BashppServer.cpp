/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "BashppServer.h"

std::mutex BashppServer::cout_mutex;

BashppServer::BashppServer() {}
BashppServer::~BashppServer() {}

void BashppServer::processMessage(const std::string& message) {
	nlohmann::json data;
	std::function<nlohmann::json(const nlohmann::json&)> handler = nullptr; // Function pointer to be set to the appropriate handler
	try {
		data = nlohmann::json::parse(message);
		const auto& method = data["method"];

		auto it = handlers.find(method);
		if (it == handlers.end()) {
			log_file << "No handler found for method: " << method << std::endl;
			return;
		}
		handler = it->second; // Set the handler to the appropriate function
	} catch (const std::exception& e) {
		std::cerr << "Error parsing message: " << e.what() << std::endl;
		return;
	}

	if (!handler) {
		log_file << "No handler found for method: " << data["method"] << std::endl;
		return;
	}

	// Call the handler and get the result
	nlohmann::json result;
	try {
		result = handler(data["params"]);
	} catch (const std::exception& e) {
		std::cerr << "Error handling message: " << e.what() << std::endl;
		result = {
			{"jsonrpc", "2.0"},
			{"id", data["id"]},
			{"error", {
				{"code", -32603}, // Internal error code
				{"message", "Internal error"},
				{"data", e.what()}
			}}
		};
	}
	// Prepare the response
	nlohmann::json response = {
		{"jsonrpc", "2.0"},
		{"id", data["id"]},
		{"result", result}
	};
	// Convert the response to a string and send it
	std::string response_str = response.dump();
	std::string header = "Content-Length: " + std::to_string(response_str.size()) + "\r\n\r\n";
	std::lock_guard<std::mutex> lock(cout_mutex); // Ensure thread-safe output
	std::cout << header << response_str << std::flush;
	log_file << "Sent response: " << response_str << std::endl;
}

json BashppServer::shutdown(const json& params) {
	json response;
	response = {
		{"jsonrpc", "2.0"},
		{"id", params["id"]},
		{"result", nullptr}
	};
	std::string response_str = response.dump();
	std::string header = "Content-Length: " + std::to_string(response_str.size()) + "\r\n\r\n";
	std::cout << header << response_str << std::flush;
	exit(0);
}

json BashppServer::handleInitialize(const json& params) {
	json result;
	result = {
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
	};
	return result;
}

json BashppServer::handleGotoDefinition(const json& params) {
	// Placeholder for actual implementation
	json result = {
		{"uri", "file:///usr/lib/bpp/stdlib/SharedStack"},
		{"range", {
			{"start", {{"line", 72}, {"character", 1}}},
			{"end", {{"line", 72}, {"character", 8}}}
		}}
	};
	return result;
}

json BashppServer::handleCompletion(const json& params) {
	// Placeholder for actual implementation
	json result = {
		{"isIncomplete", false},
		{"items", {
			{
				{"label", "example"},
				{"kind", 1}, // Text
				{"detail", "Example completion item"},
				{"documentation", "This is an example completion item."}
			}
		}}
	};
	return result;
}

json BashppServer::handleHover(const json& params) {
	// Placeholder for actual implementation
	json result = {
		{"contents", {
			{"kind", "markdown"},
			{"value", "This is a hover message."}
		}}
	};
	return result;
}

json BashppServer::handleDocumentSymbol(const json& params) {
	// Placeholder for actual implementation
	json result = {
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
	};
	return result;
}

json BashppServer::handleDidOpen(const json& params) {
	// Placeholder for actual implementation
	json result = {
		{"uri", params["textDocument"]["uri"]},
		{"languageId", "bashpp"},
		{"version", 1},
		{"text", params["textDocument"]["text"]}
	};
	return result;
}

json BashppServer::handleDidChange(const json& params) {
	// Placeholder for actual implementation
	json result = {
		{"uri", params["textDocument"]["uri"]},
		{"version", params["textDocument"]["version"]},
		{"contentChanges", params["contentChanges"]}
	};
	return result;
}

json BashppServer::handleRename(const json& params) {
	// Placeholder for actual implementation
	json result = {
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
	};
	return result;
}
