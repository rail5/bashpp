/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SemanticTokenCollector.h"

#include <algorithm>
#include <utility>

#include <bpp_include/bpp_class.h>
#include <bpp_include/bpp_datamember.h>
#include <bpp_include/bpp_method.h>
#include <bpp_include/bpp_object.h>
#include <lsp/include/resolve_entity.h>

namespace bpp {

SemanticTokenCollector::SemanticTokenCollector(
	std::string source_file,
	std::shared_ptr<bpp_program> program
) : source_file(std::move(source_file)), program(std::move(program)) {}

void SemanticTokenCollector::add_token(
	const AST::Token<std::string>& token,
	SemanticTokenType type,
	uint32_t modifiers
) {
	const std::string& value = token.getValue();
	if (value.empty()) return;

	BashppSemanticToken semantic_token{
		.line = token.getLine(),
		.start_character = token.getCharPositionInLine(),
		.length = static_cast<uint32_t>(value.length()),
		.type = type,
		.modifiers = modifiers
	};

	auto existing = std::find_if(tokens.begin(), tokens.end(),
		[&semantic_token](const BashppSemanticToken& candidate) {
			return candidate.line == semantic_token.line
				&& candidate.start_character == semantic_token.start_character
				&& candidate.length == semantic_token.length;
		}
	);

	if (existing == tokens.end()) {
		tokens.push_back(semantic_token);
		return;
	}

	const bool existing_is_declaration = (existing->modifiers & declaration_modifier) != 0;
	const bool new_is_declaration = (modifiers & declaration_modifier) != 0;
	if (new_is_declaration && !existing_is_declaration) {
		*existing = semantic_token;
	}
}

SemanticTokenType SemanticTokenCollector::classify_reference(
	const AST::Token<std::string>& token,
	SemanticTokenType fallback
) const {
	std::shared_ptr<bpp_entity> entity;
	try {
		entity = resolve_entity_at(
			source_file,
			token.getLine(),
			token.getCharPositionInLine(),
			program
		);
	} catch (...) {
		return fallback;
	}

	if (std::dynamic_pointer_cast<bpp_method>(entity)) {
		return SemanticTokenType::Method;
	}
	if (std::dynamic_pointer_cast<bpp_datamember>(entity)) {
		return SemanticTokenType::Property;
	}
	if (std::dynamic_pointer_cast<bpp_method_parameter>(entity)) {
		return SemanticTokenType::Parameter;
	}
	if (std::dynamic_pointer_cast<bpp_class>(entity)) {
		return SemanticTokenType::Class;
	}
	if (std::dynamic_pointer_cast<bpp_object>(entity)) {
		return SemanticTokenType::Variable;
	}
	return fallback;
}

void SemanticTokenCollector::enterClassDefinition(
	const std::shared_ptr<AST::ClassDefinition>& node
) {
	add_token(node->CLASSNAME(), SemanticTokenType::Class, declaration_modifier);
	if (node->PARENTCLASSNAME().has_value()) {
		add_token(*node->PARENTCLASSNAME(), SemanticTokenType::Class);
	}
}

void SemanticTokenCollector::enterMethodDefinition(
	const std::shared_ptr<AST::MethodDefinition>& node
) {
	add_token(node->NAME(), SemanticTokenType::Method, declaration_modifier);

	for (const auto& parameter_token : node->PARAMETERS()) {
		const auto& parameter = parameter_token.getValue();
		if (parameter.type.has_value()) {
			add_token(*parameter.type, SemanticTokenType::Class);
		}
		add_token(parameter.name, SemanticTokenType::Parameter, declaration_modifier);
	}
}

void SemanticTokenCollector::enterDatamemberDeclaration(
	const std::shared_ptr<AST::DatamemberDeclaration>& node
) {
	datamember_declaration_depth++;
	if (node->TYPE().has_value()) {
		add_token(*node->TYPE(), SemanticTokenType::Class);
	}
	if (node->IDENTIFIER().has_value()) {
		add_token(*node->IDENTIFIER(), SemanticTokenType::Property, declaration_modifier);
	}
}

void SemanticTokenCollector::exitDatamemberDeclaration(
	const std::shared_ptr<AST::DatamemberDeclaration>& /*node*/
) {
	datamember_declaration_depth--;
}

void SemanticTokenCollector::enterObjectInstantiation(
	const std::shared_ptr<AST::ObjectInstantiation>& node
) {
	add_token(node->TYPE(), SemanticTokenType::Class);
	add_token(
		node->IDENTIFIER(),
		datamember_declaration_depth > 0
			? SemanticTokenType::Property
			: SemanticTokenType::Variable,
		declaration_modifier
	);
}

void SemanticTokenCollector::enterPointerDeclaration(
	const std::shared_ptr<AST::PointerDeclaration>& node
) {
	add_token(node->TYPE(), SemanticTokenType::Class);
	add_token(
		node->IDENTIFIER(),
		datamember_declaration_depth > 0
			? SemanticTokenType::Property
			: SemanticTokenType::Variable,
		declaration_modifier
	);
}

void SemanticTokenCollector::enterObjectReference(
	const std::shared_ptr<AST::ObjectReference>& node
) {
	if (!node->isSelfReference()) {
		add_token(
			node->IDENTIFIER(),
			classify_reference(node->IDENTIFIER(), SemanticTokenType::Variable)
		);
	}

	for (const auto& identifier : node->IDENTIFIERS()) {
		add_token(
			identifier,
			classify_reference(identifier, SemanticTokenType::Property)
		);
	}
}

void SemanticTokenCollector::enterNewStatement(
	const std::shared_ptr<AST::NewStatement>& node
) {
	add_token(node->TYPE(), SemanticTokenType::Class);
}

void SemanticTokenCollector::enterDynamicCastTarget(
	const std::shared_ptr<AST::DynamicCastTarget>& node
) {
	if (node->TARGETTYPE().has_value()) {
		add_token(*node->TARGETTYPE(), SemanticTokenType::Class);
	}
}

std::vector<uint32_t> SemanticTokenCollector::encode() const {
	std::vector<BashppSemanticToken> sorted_tokens = tokens;
	std::ranges::sort(sorted_tokens, [](const auto& left, const auto& right) {
		if (left.line != right.line) return left.line < right.line;
		return left.start_character < right.start_character;
	});

	std::vector<uint32_t> result;
	result.reserve(sorted_tokens.size() * 5);

	uint32_t previous_line = 0;
	uint32_t previous_start_character = 0;
	bool first_token = true;

	for (const auto& token : sorted_tokens) {
		const uint32_t delta_line = first_token ? token.line : token.line - previous_line;
		const uint32_t delta_start = first_token || delta_line > 0
			? token.start_character
			: token.start_character - previous_start_character;

		result.push_back(delta_line);
		result.push_back(delta_start);
		result.push_back(token.length);
		result.push_back(static_cast<uint32_t>(token.type));
		result.push_back(token.modifiers);

		previous_line = token.line;
		previous_start_character = token.start_character;
		first_token = false;
	}

	return result;
}

} // namespace bpp
