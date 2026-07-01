/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <filesystem>
#include <error/detail.h>
#include <error/WarningOptions.h>

#include <AST/Listener/Listener.h>

#include <IR/bpp.h>

namespace bpp::AST {
class Listener;
} // namespace bpp::AST

namespace bpp::ErrorHandling {

enum class DiagnosticType : std::uint8_t {
	DIAGNOSTIC_ERROR,
	DIAGNOSTIC_WARNING,
	DIAGNOSTIC_INFO,
	DIAGNOSTIC_HINT
};

class Diagnostic {
	protected:
		/**
		 * @brief The chain of includes leading to the file which produced this diagnostic.
		 * The last element of the chain is the file which produced the diagnostic, and the first element is the original source file.
		 */
		std::vector<std::filesystem::path> include_chain;
		std::uint32_t line = 0;
		std::uint32_t column = 0;

		DiagnosticType type = DiagnosticType::DIAGNOSTIC_ERROR;

		/// If this is a warning, which -W flag enables it?
		std::optional<std::string> warning_cli_string = std::nullopt;

		/// The length of the text that should be highlighted in the source code when displaying this diagnostic
		std::uint32_t text_length = 0;
		
		/**
		 * @brief The Program that produced this diagnostic.
		 * This is used to add the diagnostic to the program's diagnostics list for language server support.
		 */
		std::shared_ptr<bpp::IR::Program> program;

		/// Any accompanying message (error or warning) to display to the user
		std::string message;

		/// Whether this diagnostic is being generated in language server mode (i.e., the output should be sent to the language server rather than printed to stderr)
		bool lsp_mode = false;

		template <bpp::detail::ASTNodePtrORToken T>
		void set_from_listener(bpp::AST::Listener* listener, const T& error_ctx) {
			include_chain = listener->get_include_chain();
			program = listener->get_program();
			//lsp_mode = listener->get_lsp_mode();

			text_length = 1;
			if constexpr (bpp::detail::ASTNodePtrType<T>) {
				line = error_ctx->getPosition().line;
				column = error_ctx->getPosition().column;
				if (error_ctx->getEndPosition().line == error_ctx->getPosition().line) {
					text_length = error_ctx->getEndPosition().column - error_ctx->getPosition().column;
				} else {
					text_length = UINT32_MAX; // Highlight the entire line if the error spans multiple lines
				}
			} else if constexpr(bpp::detail::ASTStringToken<T>) {
				line = error_ctx.getLine();
				column = error_ctx.getCharPositionInLine();
				text_length = static_cast<std::uint32_t>(error_ctx.getValue().length());
			} else if constexpr(bpp::detail::ASTParameterToken<T>) {
				// Special case: Error reporting on a declared method parameter
				// TODO(@rail5): Kind of hacky to handle special cases. Would prefer a general solution.
				line = error_ctx.getLine();
				column = error_ctx.getCharPositionInLine();
				auto param = error_ctx.getValue();
				text_length = 0;
				if (param.type.has_value()) {
					text_length += 1 + static_cast<std::uint32_t>(param.type.value().getValue().length()); // '@Type'
					if (param.pointer) text_length += 1; // '*'
					text_length += 1; // ' '
				}

				text_length = static_cast<std::uint32_t>(param.name.getValue().length()); // 'Name'
			}
		}

		void set_explicitly(
			std::vector<std::filesystem::path> include_chain,
			std::uint32_t line,
			std::uint32_t column,
			std::uint32_t text_length,
			std::string message,
			bool lsp_mode
		) {
			this->include_chain = std::move(include_chain);
			this->line = line;
			this->column = column;
			this->text_length = text_length;
			this->message = std::move(message);
			this->lsp_mode = lsp_mode;
		}
	public:
		Diagnostic() = delete;

		template <bpp::detail::ASTNodePtrORToken T>
		Diagnostic(bpp::AST::Listener* listener, const T& error_ctx, const std::string& msg) : message(msg) {
			set_from_listener(listener, error_ctx);
		}

		Diagnostic(
			std::vector<std::filesystem::path> include_chain,
			std::uint32_t line,
			std::uint32_t column,
			std::uint32_t text_length,
			const std::string& message,
			bool lsp_mode = false
		) {
			set_explicitly(std::move(include_chain), line, column, text_length, message, lsp_mode);
		}
		
		void print() const;
};

/**
 * @class SyntaxError
 * @brief An exception thrown when a syntax error is encountered
 * This exception can be constructed from any AST node or token that satisfies the ASTNodePtrORToken concept.
 * When thrown, the exception can be caught and printed to display a formatted syntax error message.
 * 
 */
class SyntaxError : public Diagnostic, public std::runtime_error {
	public:
		SyntaxError() = delete;
		explicit SyntaxError(const std::string& msg) = delete;

		template <bpp::detail::ASTNodePtrORToken T>
		SyntaxError(bpp::AST::Listener* listener, const T& error_ctx, const std::string& msg)
			: Diagnostic(listener, error_ctx, msg), std::runtime_error(msg) {}
};

class ParserError : public Diagnostic {
	public:
		ParserError() = delete;

		ParserError(
			std::vector<std::filesystem::path> include_chain,
			std::uint32_t line,
			std::uint32_t column,
			std::uint32_t text_length,
			const std::string& message,
			bool lsp_mode = false
		) : Diagnostic(std::move(include_chain), line, column, text_length, message, lsp_mode) {}
};

/**
 * @class Warning
 * @brief A compiler warning that is not fatal to compilation
 * This type should never be thrown. Instead, it should be constructed and displayed via the print() method.
 * Throwing it would halt compilation, which is not desired for warnings.
 * This likewise can be constructed from any AST node or token that satisfies the ASTNodePtrORToken concept.
 * 
 */
class Warning : public Diagnostic {
	public:
		Warning() = delete;
		explicit Warning(const std::string& msg) = delete;

		template <bpp::detail::ASTNodePtrORToken T>
		Warning(bpp::AST::Listener* listener, const T& error_ctx, const std::string& msg, WarningType warning_type)
			: Diagnostic(listener, error_ctx, msg)
		{
			this->type = DiagnosticType::DIAGNOSTIC_WARNING;
			this->warning_cli_string = listener->get_warning_options().get_cli_string_by_option(warning_type);
		}
};

// Helper functions
// Should probably be moved to a separate file or somehow better organized
std::string utf8_substr(const std::string& str, std::uint32_t start, std::uint32_t length);
std::uint32_t utf8_length(const std::string& str);
std::string equal_width_padding(const std::string& str, char padding_char = ' ');

} // namespace bpp::ErrorHandling
