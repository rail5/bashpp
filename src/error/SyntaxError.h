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
#include <error/ParserError.h>
#include <error/WarningOptions.h>

#include <AST/Listener/Listener.h>

#include <IR/bpp.h>

namespace bpp::AST {
class Listener;
} // namespace bpp::AST

namespace bpp::ErrorHandling {

/**
 * @brief Print a syntax error or warning message to stderr
 * @param line The line number where the error occurred
 * @param column The column number where the error occurred
 * @param text_length The length of the token which caused the error
 * @param msg The error message to display
 * @param include_chain A stack of include files which led to the error
 * @param warning_type The type of warning, if this is a warning; std::nullopt if this is an error
 * @param program The program being compiled, to which the error/warning should be added as a diagnostic
 */
void print_syntax_error_or_warning(
	std::uint32_t line,
	std::uint32_t column,
	std::uint32_t text_length,
	const std::string& msg,
	std::vector<std::filesystem::path> include_chain,
	std::shared_ptr<bpp::IR::Program> program,
	bool lsp_mode,
	std::optional<WarningType> warning_type,
	const std::optional<std::string>& warning_cli_string
);

void print_parser_errors(
	const std::vector<AST::ParserError>& errors,
	const std::vector<std::filesystem::path>& include_chain,
	std::shared_ptr<bpp::IR::Program> program,
	bool lsp_mode
);

class ErrorOrWarning {
	protected:
		std::uint32_t line = 0;
		std::uint32_t column = 0;
		std::uint32_t text_length = 0;
		std::vector<std::filesystem::path> include_chain;
		std::shared_ptr<bpp::IR::Program> program;
		std::string message;
		bool lsp_mode = false;
		std::optional<WarningType> warning_type = std::nullopt; // std::nullopt if this is an error, otherwise the type of warning
		std::optional<std::string> warning_cli_string = std::nullopt; // The CLI string for the warning, if this is a warning

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
					text_length += 1 + static_cast<std::uint32_t>(param.name.getValue().length()); // ' Name'
				} else {
					text_length = static_cast<std::uint32_t>(param.name.getValue().length()); // 'Name'
				}
			}
		}
	public:
		ErrorOrWarning() = delete;
		explicit ErrorOrWarning(const std::string& msg) = delete;

		template <bpp::detail::ASTNodePtrORToken T>
		ErrorOrWarning(bpp::AST::Listener* listener, const T& error_ctx, const std::string& msg) : message(msg) {
			set_from_listener(listener, error_ctx);
		}
		
		void print() const {
			print_syntax_error_or_warning(
				line,
				column,
				text_length,
				message,
				include_chain,
				program,
				lsp_mode,
				warning_type,
				warning_cli_string
			);
		}
};

/**
 * @class SyntaxError
 * @brief An exception thrown when a syntax error is encountered
 * This exception can be constructed from any AST node or token that satisfies the ASTNodePtrORToken concept.
 * When thrown, the exception can be caught and printed to display a formatted syntax error message.
 * 
 */
class SyntaxError : public ErrorOrWarning, public std::runtime_error {
	public:
		SyntaxError() = delete;
		explicit SyntaxError(const std::string& msg) = delete;

		template <bpp::detail::ASTNodePtrORToken T>
		SyntaxError(bpp::AST::Listener* listener, const T& error_ctx, const std::string& msg)
			: ErrorOrWarning(listener, error_ctx, msg), std::runtime_error(msg) {}
};

/**
 * @class Warning
 * @brief A compiler warning that is not fatal to compilation
 * This type should never be thrown. Instead, it should be constructed and displayed via the print() method.
 * Throwing it would halt compilation, which is not desired for warnings.
 * This likewise can be constructed from any AST node or token that satisfies the ASTNodePtrORToken concept.
 * 
 */
class Warning : public ErrorOrWarning {
	public:
		Warning() = delete;
		explicit Warning(const std::string& msg) = delete;

		template <bpp::detail::ASTNodePtrORToken T>
		Warning(bpp::AST::Listener* listener, const T& error_ctx, const std::string& msg, WarningType warning_type)
			: ErrorOrWarning(listener, error_ctx, msg)
		{
			this->warning_type = warning_type;
			this->warning_cli_string = listener->get_warning_options().get_cli_string_by_option(warning_type);
		}
};

// Helper functions
// Should probably be moved to a separate file or somehow better organized
std::string utf8_substr(const std::string& str, std::uint32_t start, std::uint32_t length);
std::uint32_t utf8_length(const std::string& str);
std::string equal_width_padding(const std::string& str, char padding_char = ' ');

} // namespace bpp::ErrorHandling
