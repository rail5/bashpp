/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/CompletionRequest.h"
#include "../include/resolve_entity.h"

GenericResponseMessage bpp::BashppServer::handleCompletion(const GenericRequestMessage& request) {

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

	// Wait for stored_changes_content_updating to be false before proceeding
	// Just to make sure we're suggesting completions based on the latest content
	// When the user types '.', the client will send a didChange notification,
	// and then immediately after, it'll send a completion request.
	// We need to ensure that our internally-stored version of the file content
	// is up-to-date before we resolve the reference and provide completions.
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	} while (program_pool.is_currently_storing_unsaved_changes());
	
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

	std::string uri = params.textDocument.uri;
	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		throw std::runtime_error("Ignoring request to provide completions for non-local file: " + uri);
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri, true); // Jump the queue and get the program immediately
	if (program == nullptr) {
		throw std::runtime_error("Program not found for URI: " + uri);
	}

	Position position = params.position;
	
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

	return completion_list;
}

CompletionList bpp::BashppServer::handleDOTCompletion(const CompletionParams& params) {
	// Handle '.' completions
	// Suggest method names and data members of the current object
	CompletionList completion_list;

	std::string uri = params.textDocument.uri;
	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		throw std::runtime_error("Ignoring request to provide completions for non-local file: " + uri);
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri, true); // Jump the queue and get the program immediately
	if (program == nullptr) {
		throw std::runtime_error("Program not found for URI: " + uri);
	}

	Position position = params.position;

	// Resolve the referenced entity before the dot
	std::shared_ptr<bpp::bpp_entity> referenced_entity = resolve_entity_at(
		uri,
		position.line,
		position.character - 1, // Position before the dot
		program
	);

	if (referenced_entity == nullptr) {
		throw std::runtime_error("No entity found at position: (" + std::to_string(position.line) + ", " + std::to_string(position.character) + ") in URI: " + uri);
	}

	// Ensure that the referenced entity is a non-primitive object
	std::shared_ptr<bpp::bpp_object> obj = std::dynamic_pointer_cast<bpp::bpp_object>(referenced_entity);
	if (obj == nullptr || obj->get_class() == program->get_primitive_class()) {
		throw std::runtime_error("Referenced entity is not a valid object or is a primitive type at position: (" + std::to_string(position.line) + ", " + std::to_string(position.character) + ") in URI: " + uri);
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
			case bpp::bpp_scope::SCOPE_INACCESSIBLE:
				detail += "@private ";
				break;
			case bpp::bpp_scope::SCOPE_PROTECTED:
				detail += "@protected ";
				break;
		}

		detail += "@method " + method->get_name();

		for (const auto& param : method->get_parameters()) {
			if (param->get_type() == program->get_primitive_class()) {
				detail += " $" + param->get_name();
			} else {
				detail += " @" + param->get_type()->get_name() + "* " + param->get_name();
			}
		}

		item.detail = detail;
		// Full example: @virtual @public @method methodName @ClassName* param1 $primitive_param2

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

	return completion_list;
}
