/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/HoverRequest.h>
#include <lsp/include/resolve_entity.h>

GenericResponseMessage bpp::BashppServer::handleHover(const GenericRequestMessage& request) {
	HoverRequestResponse response;
	response.id = request.id;
	HoverRequest hover_request = request.toSpecific<HoverParams>();
	std::string uri = hover_request.params.textDocument.uri;
	Position position = hover_request.params.position;

	log("Received Hover request for URI: ", uri, ", Position: (", position.line, ", ", position.character, ")");

	// Verify the URI starts with "file://"
	if (uri.find("file://") != 0) {
		log("Ignoring request to provide hover for non-local file: ", uri);
		response.result = nullptr;
		return response;
	} else {
		// Strip the "file://" prefix
		uri = uri.substr(7);
	}

	std::shared_ptr<bpp::bpp_program> program = program_pool.get_program(uri, true); // Jump the queue and get the program immediately
	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::shared_ptr<bpp::bpp_entity> entity = nullptr;
	
	try {
		entity = resolve_entity_at(
			uri,
			position.line,
			position.character,
			program
		);
	} catch (...) {
		// Ignore, it'll just be nullptr.
	}

	if (entity == nullptr) {
		log("No entity found at position: (", position.line, ", ", position.character, ") in URI: ", uri);
		response.result = nullptr;
		return response;
	}

	std::string hover_text = entity->get_name(); // Fallback in case we can't determine the type of entity

	// First, determine what kind of entity it is

	std::shared_ptr<bpp::bpp_object> obj = std::dynamic_pointer_cast<bpp::bpp_object>(entity);
	if (obj) {
		// If it's an object, display @ClassName[*] objectName
		hover_text = "@" + obj->get_class()->get_name();
		if (obj->is_pointer()) {
			hover_text += "*";
		}
		hover_text += " " + obj->get_name();
	}

	std::shared_ptr<bpp::bpp_datamember> datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity);
	if (datamember) {
		hover_text = "";
		// If it's a data member, display [@ClassName[*]] @ContainingClass.dataMemberName
		if (datamember->get_class() != program->get_primitive_class()) {
			hover_text = "@" + datamember->get_class()->get_name();
			if (datamember->is_pointer()) {
				hover_text += "*";
			}
			hover_text += " ";
		}

		std::shared_ptr<bpp::bpp_class> containing_class = datamember->get_containing_class().lock();
		if (containing_class) {
			hover_text += "@" + datamember->get_containing_class().lock()->get_name() + "." + datamember->get_name();
		} else {
			// Failsafe:
			// If we fail to lock the containing class weak ptr:
			log("Failed to lock containing class for data member: ", datamember->get_name(), " in URI: ", uri);
			hover_text += "@<error>." + datamember->get_name();
		}
	}

	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(entity);
	if (method) {
		// If it's a method, display [@virtual] {@public | @private | @protected} @method @ClassName.methodName [parameter list]
		hover_text = "";
		if (method->is_virtual()) {
			hover_text += "@virtual ";
		}

		switch (method->get_scope()) {
			case bpp::bpp_scope::SCOPE_PUBLIC:
				hover_text += "@public ";
				break;
			case bpp::bpp_scope::SCOPE_PRIVATE:
			case bpp::bpp_scope::SCOPE_INACCESSIBLE:
				hover_text += "@private ";
				break;
			case bpp::bpp_scope::SCOPE_PROTECTED:
				hover_text += "@protected ";
				break;
		}

		hover_text += "@method ";

		std::shared_ptr<bpp::bpp_class> containing_class = method->get_containing_class().lock();
		if (containing_class) {
			hover_text += "@" + containing_class->get_name();
		} else {
			// Failsafe:
			hover_text += "@<error>";
		}
		hover_text += "." + method->get_name();
		for (const auto& param : method->get_parameters()) {
			if (param->get_class() == program->get_primitive_class()) {
				hover_text += " $" + param->get_name();
			} else {
				hover_text += " @" + param->get_class()->get_name() + "* " + param->get_name();
			}
		}
	}

	std::shared_ptr<bpp::bpp_class> cls = std::dynamic_pointer_cast<bpp::bpp_class>(entity);
	if (cls) {
		// If it's a class, display @class ClassName : ParentClass
		hover_text = "@class " + cls->get_name();
		std::shared_ptr<bpp::bpp_class> parent = cls->get_parent();
		if (parent) {
			hover_text += " : " + parent->get_name();
		}
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
