/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/HoverRequest.h>
#include <lsp/include/find_comments.h>
#include <lsp/include/validateUri.h>

#include <IR/entities/Entity.h>
#include <IR/entities/Object.h>
#include <IR/entities/Class.h>
#include <IR/entities/Method.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Program.h>
#include <IR/entities/expressions/bpp/IncludeStatement.h>

GenericResponseMessage bpp::LSP::BashppServer::handleHover(const GenericRequestMessage& request) {
	HoverRequestResponse response;
	response.id = request.id;
	HoverRequest hover_request = request.toSpecific<HoverParams>();
	std::string uri = hover_request.params.textDocument.uri;
	Position position = hover_request.params.position;

	log("Received Hover request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	try {
		uri = validateUri(hover_request.params.textDocument.uri);
	} catch (const std::exception& e) {
		log("Invalid URI in Hover request: ", e.what());
		response.result = nullptr;
		return response;
	}

	auto program = program_pool.getProgram(uri, true); // Jump the queue and get the program immediately
	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.result = nullptr;
		return response;
	}

	const auto* entity = program->source_index->getReferencedEntity({uri, position.line, position.character});

	if (entity == nullptr) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::string label, explanation, code_snippet;
	if (const auto* named_entity = dynamic_cast<const bpp::IR::Components::Named*>(entity)) {
		label = "Entity `" + named_entity->getName() + "`"; // Fallback in case we can't determine the type of entity
	}

	// First, determine what kind of entity it is

	if (const auto* datamember = dynamic_cast<const bpp::IR::DataMember*>(entity)) {
		label = "Data Member `" + datamember->getName() + "`";
		// If it's a data member, display [@ClassName[*]] @ContainingClass.dataMemberName
		if (datamember->getType() != nullptr) {
			code_snippet = "@" + datamember->getType()->getName();
			if (datamember->isPointer()) code_snippet += "*";
			code_snippet += " ";

			explanation = "Type: `" + datamember->getType()->getName() + "`";
		} else {
			explanation = "Type: primitive";
		}

		const auto* containing_class = datamember->getContainingClass();
		if (containing_class) {
			code_snippet += "@" + containing_class->getName() + "." + datamember->getName();
			explanation += "\n\nProvided by class `" + containing_class->getName() + "`";
		} else {
			// Failsafe:
			// If we fail to lock the containing class weak ptr:
			log("Failed to lock containing class for data member: ", datamember->getName(), " in URI: ", uri);
			code_snippet += "@<error>." + datamember->getName();
		}

		if (const auto* parent = datamember->getParentDatamember()) {
			const auto* parent_class = parent->getContainingClass();
			if (parent_class) explanation += "\n\nInherited from class `" + parent_class->getName() + "`";

			const auto* furthest_ancestor_dm = [parent]() {
				const bpp::IR::DataMember* current = parent;
				while (current->getParentDatamember() != nullptr) current = current->getParentDatamember();
				return current;
			}();

			if (furthest_ancestor_dm && furthest_ancestor_dm != parent) {
				const auto* ancestor_class = furthest_ancestor_dm->getContainingClass();
				if (ancestor_class) explanation += "\n\nFirst defined in class `" + ancestor_class->getName() + "`";
			}
		}
	} else if (const auto* obj = dynamic_cast<const bpp::IR::Object*>(entity)) {
		// If it's an object (but not a data member), display @ClassName[*] objectName
		if (obj->getType() != nullptr) {
			label = "Object `" + obj->getName() + "`";

			code_snippet = "@" + obj->getType()->getName();
			if (obj->isPointer()) code_snippet += "*";
			code_snippet += " " + obj->getName();

			explanation = "Type: `" + obj->getType()->getName() + "`";
		} else {
			// primitive object, display $objectName
			label = "Primitive `" + obj->getName() + "`";
			explanation = "Type: primitive";
			code_snippet = "$" + obj->getName();
		}
	} else if (const auto* method = dynamic_cast<const bpp::IR::Method*>(entity)) {
		// If it's a method, display [@virtual] {@public | @private | @protected} @method @ClassName.methodName [parameter list]
		label = "Method `" + method->getName() + "`";
		if (method->isVirtual()) code_snippet = "@virtual ";

		switch (method->getScope()) {
			case bpp::IR::VisibilityScope::PUBLIC:
				code_snippet += "@public ";
				break;
			case bpp::IR::VisibilityScope::PRIVATE:
			case bpp::IR::VisibilityScope::INACCESSIBLE:
				code_snippet += "@private ";
				break;
			case bpp::IR::VisibilityScope::PROTECTED:
				code_snippet += "@protected ";
				break;
		}

		code_snippet += "@method ";

		const auto* containing_class = method->getContainingClass();
		if (containing_class) {
			code_snippet += "@" + containing_class->getName();
			explanation = "Provided by class `" + containing_class->getName() + "`";
		} else {
			// Failsafe:
			code_snippet += "@<error>";
		}
		code_snippet += "." + method->getName();

		// Get a view of the parameter list, excluding the implicit `this` parameter (which is always the first parameter)
		auto parameters = method->getParameters() | std::views::drop(1);
		for (const auto& [index, param] : parameters) {
			if (param->getType() == nullptr) {
				code_snippet += " $" + param->getName();
			} else {
				code_snippet += " @" + param->getType()->getName() + "* " + param->getName();
			}
		}

		if (const auto* parent_method = method->getParentMethod()) {
			const auto* parent_class = parent_method->getContainingClass();
			if (parent_class) explanation += "\n\nInherited from class `" + parent_class->getName() + "`";

			const auto* furthest_ancestor_method = [parent_method]() {
				const bpp::IR::Method* current = parent_method;
				while (current->getParentMethod() != nullptr) current = current->getParentMethod();
				return current;
			}();

			if (furthest_ancestor_method && furthest_ancestor_method != parent_method) {
				const auto* ancestor_class = furthest_ancestor_method->getContainingClass();
				if (ancestor_class) explanation += "\n\nFirst defined in class `" + ancestor_class->getName() + "`";
			}
		}
	} else if (const auto* cls = dynamic_cast<const bpp::IR::Class*>(entity)) {
		// If it's a class, display @class ClassName : ParentClass
		label = "Class `" + cls->getName() + "`";

		auto definition = program->source_index->getDefinitionPosition(cls);
		if (definition) explanation = "Provided by `" + definition->source_file.string() + "`";

		code_snippet = "@class " + cls->getName();
		const auto* parent = cls->getParentClass();
		if (parent) code_snippet += " : " + parent->getName();
	} else if (const auto* include_statement = dynamic_cast<const bpp::IR::IncludeStatement*>(entity)) {
		const auto& file_path = include_statement->getIncludedFilePath();
		label = "`" + file_path.filename().string() + "`";
		explanation = "`" + file_path.string() + "`";
		code_snippet.clear();
	}

	if (label.empty()) {
		response.result = nullptr;
		return response; // No hover text available
	}

	// Search for any relevant comments associated with the entity
	std::string comments = find_comments_for_entity(entity, &program_pool);

	if (!comments.empty()) {
		code_snippet += "\n\n" + comments; // Append comments if available
	}

	Hover hover;
	MarkupContent hoverContent;
	hoverContent.kind = MarkupKind::Markdown;
	hoverContent.value = std::format(
		"### {}\n\n" // Label (e.g., ### Class `someName`)
		"---\n\n" // separator line
		"{}\n\n" // Explanation section (e.g., Provided by `/path/to/file.bpp`)
		"---\n\n" // separator line
		"```bashpp\n"
			"{}\n" // Code snippet section (e.g., @class someName : someParent, + comment lines)
		"```",
		label, explanation, code_snippet);
	hover.contents = hoverContent;

	response.result = hover;

	return response;
}
