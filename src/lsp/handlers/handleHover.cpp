/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "../BashppServer.h"
#include "../generated/HoverRequest.h"

GenericResponseMessage bpp::BashppServer::handleHover(const GenericRequestMessage& request) {
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
