/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/CompletionRequest.h>
#include <lsp/include/validateUri.h>

#include <IR/entities/Entity.h>
#include <IR/entities/Program.h>
#include <IR/entities/Class.h>
#include <IR/entities/Object.h>
#include <IR/entities/Method.h>
#include <IR/entities/DataMember.h>

GenericResponseMessage bpp::BashppServer::handleCompletion(const GenericRequestMessage& request) {
	CompletionRequestResponse response;
	response.id = request.id;
	CompletionRequest completion_request = request.toSpecific<CompletionParams>();

	std::string uri = completion_request.params.textDocument.uri;

	try {
		uri = validateUri(uri);
	} catch (const std::exception& e) {
		log("Invalid URI in completion request: ", e.what());
		response.result = nullptr;
		return response;
	}

	// Wait for processing_didChange to be false before proceeding
	// Just to make sure we're suggesting completions based on the latest content
	// When the user types '.', the client will send a didChange notification,
	// and then immediately after, it'll send a completion request.
	// We need to ensure that our internally-stored version of the file content
	// is up-to-date before we resolve the reference and provide completions.
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	} while (processing_didChange.load(std::memory_order_acquire));

	// Which character triggered the request?
	char trigger_character = '.';
	if (completion_request.params.context.has_value() && completion_request.params.context->triggerCharacter.has_value()) {
		trigger_character = completion_request.params.context->triggerCharacter.value()[0];
	}

	CompletionList completion_list;

	// If it was '@', suggest class names, object names, and standard operators like include or dynamic_cast
	// If it was '.', suggest method names and data members of the current object
	// Also, '.' requires for us to wait until previous unsaved changes have been stored (done by default)
	switch (trigger_character) {
		case '@':
		default:
			log("Received Completion request for URI: ", uri, " with trigger character: '@'");
			try {
				completion_list = handleATCompletion(completion_request.params);
			} catch (const std::exception& e) {
				log(e.what());
				response.result = nullptr;
				return response;
			}
		break;
		case '.':
			log("Received Completion request for URI: ", uri, " with trigger character: '.'");
			try {
				completion_list = handleDOTCompletion(completion_request.params);
			} catch (const std::exception& e) {
				log(e.what());
				response.result = nullptr;
				return response;
			}
		break;
	}

	response.result = completion_list;
	return response;
}

CompletionList bpp::BashppServer::handleATCompletion(const CompletionParams& params) {
	// Handle '@' completions
	// Suggest class names, object names, and standard operators like include or dynamic_cast
	CompletionList completion_list = default_completion_list; // Start with the default completion list

	std::string uri = validateUri(params.textDocument.uri);

	auto program = program_pool.getProgram(uri, true); // Jump the queue and get the program immediately
	if (program == nullptr) throw std::runtime_error("Program not found for URI: " + uri);

	Position position = params.position;

	// Which context entity is active at the given position?
	const auto* active_entity = program->source_index->getContextEntity({uri, position.line, position.character});
	if (active_entity == nullptr) {
		log("BUG: No context entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri, " - returning default completions.");
	} else {
		const auto& classes = active_entity->getAllKnownClasses();

		// Collect objects both foreign to & owned by the active entity into a single 'objects' map
		const auto& objects = active_entity->getAllKnownObjects();

		for (const auto* obj : objects) {
			if (obj == nullptr) continue;
			if (obj->getType() == nullptr) continue; // Skip primitive objects
			CompletionItem item;
			item.label = obj->getName();
			item.kind = CompletionItemKind::Variable;
			item.detail = "@" + obj->getType()->getName() + " " + obj->getName(); // As in: @ClassName objectName
			completion_list.items.push_back(item);
		}

		for (const auto& cls : classes) {
			if (cls == nullptr) continue;
			CompletionItem item;
			item.label = cls->getName();
			item.kind = CompletionItemKind::Class;
			item.detail = "@class " + cls->getName(); // As in: @class ClassName
			completion_list.items.push_back(item);
		}
	}

	return completion_list;
}

CompletionList bpp::BashppServer::handleDOTCompletion(const CompletionParams& params) {
	// Handle '.' completions
	// Suggest method names and data members of the current object
	CompletionList completion_list;

	std::string uri = validateUri(params.textDocument.uri);

	auto program = program_pool.getProgram(uri, true); // Jump the queue and get the program immediately
	if (program == nullptr) throw std::runtime_error("Program not found for URI: " + uri);

	Position position = params.position;

	// Resolve the referenced entity before the dot
	const auto* referenced_entity = program->source_index->getReferencedEntity({uri, position.line, position.character - 1}); // Position before the dot

	if (referenced_entity == nullptr) {
		throw std::runtime_error("No entity found at position: (" + std::to_string(position.line) + ", " + std::to_string(position.character) + ") in URI: " + uri);
	}

	// Ensure that the referenced entity is a non-primitive object (@obj) or a class (e.g., @this/@super)
	const auto* obj = dynamic_cast<const bpp::IR::Object*>(referenced_entity);
	const auto* cls = dynamic_cast<const bpp::IR::Class*>(referenced_entity);

	const bpp::IR::Class* entity_class = nullptr;
	std::string entity_name;

	if (obj != nullptr && obj->getType() != nullptr) {
		entity_class = obj->getType();
		entity_name = obj->getName();
	} else if (cls != nullptr) {
		entity_class = cls;
		entity_name = cls->getName();
	} else {
		throw std::runtime_error("Referenced entity is not a valid object or class at position: (" + std::to_string(position.line) + ", " + std::to_string(position.character) + ") in URI: " + uri);
	}

	for (const auto& method : entity_class->getAllMethods()) {
		if (method->getName().contains("__")) {
			continue; // Skip system methods
		}

		CompletionItem item;
		item.label = method->getName();
		item.kind = CompletionItemKind::Method;

		std::string detail;

		if (method->isVirtual()) {
			detail += "@virtual ";
		}

		switch (method->getScope()) {
			case bpp::IR::VisibilityScope::PUBLIC:
				detail += "@public ";
				break;
			case bpp::IR::VisibilityScope::PRIVATE:
			case bpp::IR::VisibilityScope::INACCESSIBLE:
				detail += "@private ";
				break;
			case bpp::IR::VisibilityScope::PROTECTED:
				detail += "@protected ";
				break;
		}

		detail += "@method " + method->getName();

		for (const auto& [index, param] : method->getParameters()) {
			if (param->getType() == nullptr) {
				detail += " $" + param->getName();
			} else {
				detail += " @" + param->getType()->getName() + "* " + param->getName();
			}
		}

		item.detail = detail;
		// Full example: @virtual @public @method methodName @ClassName* param1 $primitive_param2

		completion_list.items.push_back(item);
	}

	for (const auto& data_member : entity_class->getAllDatamembers()) {
		CompletionItem item;
		item.label = data_member->getName();
		item.kind = CompletionItemKind::Field;

		std::string detail = "@" + entity_name + "." + data_member->getName();
		detail += " (";

		if (data_member->getType() == nullptr) {
			detail += "primitive";
			if (data_member->isArray()) detail += " array";
		} else {
			detail += "@" + data_member->getType()->getName();

			if (data_member->isPointer()) detail += "*";
		}
		detail += ")";

		item.detail = detail;
		// Full example: @objectName.dataMemberName (primitive)
		// Or: @objectName.dataMemberName (primitive array)
		// Or: @objectName.dataMemberName (@ClassName)
		// Or: @objectName.dataMemberName (@ClassName*)

		completion_list.items.push_back(item);
	}

	return completion_list;
}
