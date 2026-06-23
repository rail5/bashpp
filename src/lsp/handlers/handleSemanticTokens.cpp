/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <lsp/BashppServer.h>
#include <lsp/generated/SemanticTokensRequest.h>
#include <lsp/include/SemanticTokenCollector.h>
#include <lsp/include/validateUri.h>

GenericResponseMessage bpp::BashppServer::handleSemanticTokens(
	const GenericRequestMessage& request
) {
	SemanticTokensRequestResponse response;
	response.id = request.id;
	SemanticTokensRequest semantic_tokens_request =
		request.toSpecific<SemanticTokensParams>();

	std::string uri;
	try {
		uri = validateUri(semantic_tokens_request.params.textDocument.uri);
	} catch (const std::exception& exception) {
		log("Invalid URI in semantic tokens request: ", exception.what());
		response.result = nullptr;
		return response;
	}

	auto program = program_pool.get_program(uri);
	if (program == nullptr) {
		log("Program not found for URI: ", uri);
		response.result = nullptr;
		return response;
	}

	auto ast = program->get_source_file_ast(uri);
	if (ast == nullptr) {
		log("AST not found for URI: ", uri);
		response.result = nullptr;
		return response;
	}

	SemanticTokenCollector collector(
		uri,
		program,
		program_pool.get_lexer_tokens(uri),
		program_pool.get_utf16_mode()
	);
	collector.walk(ast);

	SemanticTokens result;
	result.data = collector.encode();
	response.result = result;
	return response;
}
