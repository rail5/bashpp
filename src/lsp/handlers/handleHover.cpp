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

GenericResponseMessage bpp::BashppServer::handleHover(const GenericRequestMessage& request) {
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

	std::string hover_text; // Fallback in case we can't determine the type of entity
	if (const auto* named_entity = dynamic_cast<const bpp::IR::Components::Named*>(entity)) {
		hover_text = named_entity->getName();
	}

	// First, determine what kind of entity it is

	const auto* datamember = dynamic_cast<const bpp::IR::DataMember*>(entity);
	if (datamember) {
		hover_text = "";
		// If it's a data member, display [@ClassName[*]] @ContainingClass.dataMemberName
		if (datamember->getType() != nullptr) {
			hover_text = "@" + datamember->getType()->getName();
			if (datamember->isPointer()) hover_text += "*";
			hover_text += " ";
		}

		const auto* containing_class = datamember->getContainingClass();
		if (containing_class) {
			hover_text += "@" + containing_class->getName() + "." + datamember->getName();
		} else {
			// Failsafe:
			// If we fail to lock the containing class weak ptr:
			log("Failed to lock containing class for data member: ", datamember->getName(), " in URI: ", uri);
			hover_text += "@<error>." + datamember->getName();
		}
	}

	const auto* obj = dynamic_cast<const bpp::IR::Object*>(entity);
	if (obj) {
		// If it's an object, display @ClassName[*] objectName
		if (obj->getType() != nullptr) {
			hover_text = "@" + obj->getType()->getName();
			if (obj->isPointer()) hover_text += "*";
			hover_text += " " + obj->getName();
		}
	}

	const auto* method = dynamic_cast<const bpp::IR::Method*>(entity);
	if (method) {
		// If it's a method, display [@virtual] {@public | @private | @protected} @method @ClassName.methodName [parameter list]
		hover_text = "";
		if (method->isVirtual()) hover_text += "@virtual ";

		switch (method->getScope()) {
			case bpp::IR::VisibilityScope::PUBLIC:
				hover_text += "@public ";
				break;
			case bpp::IR::VisibilityScope::PRIVATE:
			case bpp::IR::VisibilityScope::INACCESSIBLE:
				hover_text += "@private ";
				break;
			case bpp::IR::VisibilityScope::PROTECTED:
				hover_text += "@protected ";
				break;
		}

		hover_text += "@method ";

		const auto* containing_class = method->getContainingClass();
		if (containing_class) {
			hover_text += "@" + containing_class->getName();
		} else {
			// Failsafe:
			hover_text += "@<error>";
		}
		hover_text += "." + method->getName();
		for (const auto& [index, param] : method->getParameters()) {
			if (param->getType() == nullptr) {
				hover_text += " $" + param->getName();
			} else {
				hover_text += " @" + param->getType()->getName() + "* " + param->getName();
			}
		}
	}

	const auto* cls = dynamic_cast<const bpp::IR::Class*>(entity);
	if (cls) {
		// If it's a class, display @class ClassName : ParentClass
		hover_text = "@class " + cls->getName();
		const auto* parent = cls->getParentClass();
		if (parent) hover_text += " : " + parent->getName();
	}

	if (hover_text.empty()) {
		response.result = nullptr;
		return response; // No hover text available
	}

	// Search for any relevant comments associated with the entity
	std::string comments = find_comments_for_entity(entity, &program_pool);

	if (!comments.empty()) {
		hover_text += "\n\n" + comments; // Append comments if available
	}

	Hover hover;
	MarkupContent hoverContent;
	hoverContent.kind = MarkupKind::Markdown;
	hoverContent.value = "```bashpp\n" + hover_text + "\n```";
	hover.contents = hoverContent;

	response.result = hover;

	return response;
}
