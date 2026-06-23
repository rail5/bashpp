/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SemanticTokenCollector.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>
#include <utility>

#include <bpp_include/bpp_class.h>
#include <bpp_include/bpp_datamember.h>
#include <bpp_include/bpp_method.h>
#include <bpp_include/bpp_object.h>
#include <lsp/include/resolve_entity.h>

namespace bpp {

namespace {

std::optional<SemanticTokenType> classify_lexer_token(std::string_view kind) {
	if (kind == "COMMENT") return SemanticTokenType::Comment;
	if (kind == "INTEGER") return SemanticTokenType::Number;
	if (kind == "BASH_VAR") return SemanticTokenType::Variable;
	if (kind == "IDENTIFIER_LVALUE" || kind == "BASH_FUNCTION_LABEL") {
		return SemanticTokenType::Function;
	}
	if (kind.starts_with("KEYWORD_") || kind.starts_with("BASH_KEYWORD_")) {
		return SemanticTokenType::Keyword;
	}

	static constexpr std::array string_tokens{
		std::string_view("SINGLEQUOTED_STRING"),
		std::string_view("QUOTE_BEGIN"),
		std::string_view("QUOTE_END"),
		std::string_view("STRING_CONTENT"),
		std::string_view("INCLUDE_PATH")
	};
	if (std::ranges::contains(string_tokens, kind)) {
		return SemanticTokenType::String;
	}

	static constexpr std::array operator_tokens{
		std::string_view("DOUBLEAMPERSAND"),
		std::string_view("DOUBLEPIPE"),
		std::string_view("PIPE"),
		std::string_view("DELIM"),
		std::string_view("AT"),
		std::string_view("AT_LVALUE"),
		std::string_view("LBRACE"),
		std::string_view("RBRACE"),
		std::string_view("LANGLE"),
		std::string_view("RANGLE"),
		std::string_view("LANGLE_AMPERSAND"),
		std::string_view("RANGLE_AMPERSAND"),
		std::string_view("AMPERSAND_RANGLE"),
		std::string_view("COLON"),
		std::string_view("PLUS_EQUALS"),
		std::string_view("EQUALS"),
		std::string_view("ASTERISK"),
		std::string_view("DEREFERENCE_OPERATOR"),
		std::string_view("AMPERSAND"),
		std::string_view("DOT"),
		std::string_view("SUPERSHELL_START"),
		std::string_view("SUPERSHELL_END"),
		std::string_view("SUBSHELL_START"),
		std::string_view("SUBSHELL_END"),
		std::string_view("SUBSHELL_SUBSTITUTION_START"),
		std::string_view("SUBSHELL_SUBSTITUTION_END"),
		std::string_view("ARRAY_ASSIGNMENT_START"),
		std::string_view("ARRAY_ASSIGNMENT_END"),
		std::string_view("DEPRECATED_SUBSHELL_START"),
		std::string_view("DEPRECATED_SUBSHELL_END"),
		std::string_view("LPAREN"),
		std::string_view("RPAREN"),
		std::string_view("ARRAY_INDEX_START"),
		std::string_view("ARRAY_INDEX_END"),
		std::string_view("LBRACKET"),
		std::string_view("RBRACKET"),
		std::string_view("REF_START"),
		std::string_view("REF_START_LVALUE"),
		std::string_view("REF_END"),
		std::string_view("BASH_VAR_START"),
		std::string_view("BASH_VAR_END"),
		std::string_view("HASH"),
		std::string_view("HEREDOC_CONTENT_START"),
		std::string_view("HERESTRING_START"),
		std::string_view("HEREDOC_START"),
		std::string_view("BASH_CASE_PATTERN_DELIM"),
		std::string_view("BASH_CASE_PATTERN_TERMINATOR"),
		std::string_view("ARITH_FOR_CONDITION_START"),
		std::string_view("ARITH_FOR_CONDITION_END"),
		std::string_view("INCREMENT_OPERATOR"),
		std::string_view("DECREMENT_OPERATOR"),
		std::string_view("COMPARISON_OPERATOR"),
		std::string_view("BASH_FUNCTION_OPEN"),
		std::string_view("BASH_TEST_CONDITION_START"),
		std::string_view("BASH_TEST_CONDITION_END"),
		std::string_view("EXCLAM"),
		std::string_view("EXPANSION_BEGIN"),
		std::string_view("PROCESS_SUBSTITUTION_START"),
		std::string_view("PROCESS_SUBSTITUTION_END"),
		std::string_view("BASH_ARITHMETIC_START"),
		std::string_view("BASH_ARITHMETIC_END"),
		std::string_view("BASH_53_NATIVE_SUPERSHELL_START"),
		std::string_view("BASH_53_NATIVE_SUPERSHELL_END")
	};
	if (std::ranges::contains(operator_tokens, kind)) {
		return SemanticTokenType::Operator;
	}

	return std::nullopt;
}

} // namespace

SemanticTokenCollector::SemanticTokenCollector(
	std::string source_file,
	std::shared_ptr<bpp_program> program,
	const std::vector<AST::LexerToken>& lexer_tokens,
	bool utf16_mode
) :
	source_file(std::move(source_file)),
	program(std::move(program)),
	utf16_mode(utf16_mode) {
	for (const auto& token : lexer_tokens) {
		add_lexer_token(token);
	}
}

void SemanticTokenCollector::add_token(BashppSemanticToken semantic_token) {
	if (semantic_token.length == 0) return;

	auto existing = std::find_if(tokens.begin(), tokens.end(),
		[&semantic_token](const BashppSemanticToken& candidate) {
			return candidate.line == semantic_token.line
				&& candidate.start_character == semantic_token.start_character
				&& candidate.length == semantic_token.length;
		}
	);

	if (existing == tokens.end()) {
		tokens.push_back(std::move(semantic_token));
		return;
	}

	if (existing->syntax && !semantic_token.syntax) {
		*existing = std::move(semantic_token);
		return;
	}

	const bool existing_is_declaration = (existing->modifiers & declaration_modifier) != 0;
	const bool new_is_declaration =
		(semantic_token.modifiers & declaration_modifier) != 0;
	if (new_is_declaration && !existing_is_declaration) {
		*existing = std::move(semantic_token);
	}
}

void SemanticTokenCollector::add_token(
	const AST::Token<std::string>& token,
	SemanticTokenType type,
	uint32_t modifiers
) {
	const std::string& value = token.getValue();
	if (value.empty()) return;

	add_token(BashppSemanticToken{
		.line = token.getLine(),
		.start_character = token.getCharPositionInLine(),
		.length = encoded_character_count(value),
		.type = type,
		.modifiers = modifiers,
		.syntax = false
	});
}

uint32_t SemanticTokenCollector::encoded_character_count(const std::string& text) const {
	uint32_t count = 0;
	for (size_t index = 0; index < text.size();) {
		const unsigned char first = static_cast<unsigned char>(text[index]);
		if (first == '\r') {
			index++;
			continue;
		}

		size_t byte_count = 1;
		uint32_t codepoint = first;
		if ((first & 0xe0) == 0xc0 && index + 1 < text.size()) {
			byte_count = 2;
			codepoint = (first & 0x1f) << 6
				| (static_cast<unsigned char>(text[index + 1]) & 0x3f);
		} else if ((first & 0xf0) == 0xe0 && index + 2 < text.size()) {
			byte_count = 3;
			codepoint = (first & 0x0f) << 12
				| (static_cast<unsigned char>(text[index + 1]) & 0x3f) << 6
				| (static_cast<unsigned char>(text[index + 2]) & 0x3f);
		} else if ((first & 0xf8) == 0xf0 && index + 3 < text.size()) {
			byte_count = 4;
			codepoint = (first & 0x07) << 18
				| (static_cast<unsigned char>(text[index + 1]) & 0x3f) << 12
				| (static_cast<unsigned char>(text[index + 2]) & 0x3f) << 6
				| (static_cast<unsigned char>(text[index + 3]) & 0x3f);
		}

		count += utf16_mode && codepoint >= 0x10000 ? 2 : 1;
		index += byte_count;
	}
	return count;
}

void SemanticTokenCollector::add_lexer_token(const AST::LexerToken& token) {
	auto type = classify_lexer_token(token.kind);
	if (!type.has_value() && token.kind == "CATCHALL") {
		static constexpr std::array operator_text{
			std::string_view("!"),
			std::string_view("["),
			std::string_view("]"),
			std::string_view("+"),
			std::string_view("-"),
			std::string_view("*"),
			std::string_view("/"),
			std::string_view("%"),
			std::string_view("="),
			std::string_view("<"),
			std::string_view(">"),
			std::string_view("&"),
			std::string_view("|")
		};
		if (std::ranges::contains(operator_text, token.text)) {
			type = SemanticTokenType::Operator;
		}
	}
	if (!type.has_value() || token.text.empty()) return;

	std::string text = token.text;
	ParserPosition position = token.location.begin;
	const bool preserve_whitespace = *type == SemanticTokenType::String;
	if (!preserve_whitespace) {
		const size_t first = text.find_first_not_of(" \t\r\n");
		if (first == std::string::npos) return;

		for (size_t index = 0; index < first; index++) {
			if (text[index] == '\n') {
				position.line++;
				position.column = 0;
			} else if (text[index] != '\r') {
				position.column++;
			}
		}

		const size_t last = text.find_last_not_of(" \t\r\n");
		text = text.substr(first, last - first + 1);
	}

	size_t segment_start = 0;
	while (segment_start <= text.size()) {
		const size_t newline = text.find('\n', segment_start);
		const size_t segment_end = newline == std::string::npos
			? text.size()
			: newline;
		const std::string segment = text.substr(
			segment_start,
			segment_end - segment_start
		);
		const uint32_t length = encoded_character_count(segment);
		if (length > 0) {
			add_token(BashppSemanticToken{
				.line = position.line,
				.start_character = position.column,
				.length = length,
				.type = *type,
				.modifiers = 0,
				.syntax = true
			});
		}

		if (newline == std::string::npos) break;
		position.line++;
		position.column = 0;
		segment_start = newline + 1;
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

void SemanticTokenCollector::enterPrimitiveAssignment(
	const std::shared_ptr<AST::PrimitiveAssignment>& node
) {
	add_token(node->IDENTIFIER(), SemanticTokenType::Variable);
}

void SemanticTokenCollector::enterBashFunction(
	const std::shared_ptr<AST::BashFunction>& node
) {
	add_token(node->NAME(), SemanticTokenType::Function, declaration_modifier);
}

std::vector<uint32_t> SemanticTokenCollector::encode() const {
	std::vector<BashppSemanticToken> semantic_tokens;
	for (const auto& token : tokens) {
		if (!token.syntax) semantic_tokens.push_back(token);
	}

	std::vector<BashppSemanticToken> sorted_tokens = semantic_tokens;
	for (const auto& syntax_token : tokens) {
		if (!syntax_token.syntax) continue;

		// LSP tokens cannot overlap. Preserve AST classifications by splitting
		// broader lexer tokens into the portions that remain around them.
		std::vector<std::pair<uint32_t, uint32_t>> fragments = {{
			syntax_token.start_character,
			syntax_token.start_character + syntax_token.length
		}};

		for (const auto& semantic_token : semantic_tokens) {
			if (semantic_token.line != syntax_token.line) continue;

			const uint32_t semantic_start = semantic_token.start_character;
			const uint32_t semantic_end = semantic_start + semantic_token.length;
			std::vector<std::pair<uint32_t, uint32_t>> remaining_fragments;
			for (const auto& [fragment_start, fragment_end] : fragments) {
				if (semantic_end <= fragment_start || semantic_start >= fragment_end) {
					remaining_fragments.emplace_back(fragment_start, fragment_end);
					continue;
				}
				if (fragment_start < semantic_start) {
					remaining_fragments.emplace_back(fragment_start, semantic_start);
				}
				if (semantic_end < fragment_end) {
					remaining_fragments.emplace_back(semantic_end, fragment_end);
				}
			}
			fragments = std::move(remaining_fragments);
		}

		for (const auto& [start, end] : fragments) {
			if (start == end) continue;
			auto fragment = syntax_token;
			fragment.start_character = start;
			fragment.length = end - start;
			sorted_tokens.push_back(fragment);
		}
	}

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
