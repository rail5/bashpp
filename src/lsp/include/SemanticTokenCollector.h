/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <AST/LexerToken.h>
#include <AST/Listener/BaseListener.h>
#include <bpp_include/bpp_program.h>

namespace bpp {

enum class SemanticTokenType : uint32_t {
	Class,
	Method,
	Property,
	Variable,
	Parameter,
	Keyword,
	Comment,
	String,
	Operator,
	Number,
	Function
};

struct BashppSemanticToken {
	uint32_t line = 0;
	uint32_t start_character = 0;
	uint32_t length = 0;
	SemanticTokenType type = SemanticTokenType::Variable;
	uint32_t modifiers = 0;
	bool syntax = false;
};

class SemanticTokenCollector : public AST::BaseListener<SemanticTokenCollector> {
	private:
		static constexpr uint32_t declaration_modifier = 1;

		std::string source_file;
		std::shared_ptr<bpp_program> program;
		std::vector<BashppSemanticToken> tokens;
		size_t datamember_declaration_depth = 0;
		bool utf16_mode = false;

		void add_token(BashppSemanticToken token);
		void add_token(
			const AST::Token<std::string>& token,
			SemanticTokenType type,
			uint32_t modifiers = 0
		);
		SemanticTokenType classify_reference(
			const AST::Token<std::string>& token,
			SemanticTokenType fallback
		) const;
		void add_lexer_token(const AST::LexerToken& token);
		uint32_t encoded_character_count(const std::string& text) const;

	public:
		SemanticTokenCollector(
			std::string source_file,
			std::shared_ptr<bpp_program> program,
			const std::vector<AST::LexerToken>& lexer_tokens,
			bool utf16_mode
		);

		void enterClassDefinition(const std::shared_ptr<AST::ClassDefinition>& node);
		void enterMethodDefinition(const std::shared_ptr<AST::MethodDefinition>& node);
		void enterDatamemberDeclaration(const std::shared_ptr<AST::DatamemberDeclaration>& node);
		void exitDatamemberDeclaration(const std::shared_ptr<AST::DatamemberDeclaration>& node);
		void enterObjectInstantiation(const std::shared_ptr<AST::ObjectInstantiation>& node);
		void enterPointerDeclaration(const std::shared_ptr<AST::PointerDeclaration>& node);
		void enterObjectReference(const std::shared_ptr<AST::ObjectReference>& node);
		void enterNewStatement(const std::shared_ptr<AST::NewStatement>& node);
		void enterDynamicCastTarget(const std::shared_ptr<AST::DynamicCastTarget>& node);
		void enterPrimitiveAssignment(const std::shared_ptr<AST::PrimitiveAssignment>& node);
		void enterBashFunction(const std::shared_ptr<AST::BashFunction>& node);

		std::vector<uint32_t> encode() const;
};

} // namespace bpp
