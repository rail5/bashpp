/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <string>
#include <stack>
#include <memory>
#include <cstdint>
#include <stdexcept>
#include <bpp_include/bpp.h>
#include <error/detail.h>
#include <error/ParserError.h>

namespace bpp {
namespace ErrorHandling {

/**
 * @brief Print a syntax error or warning message to stderr
 * @param source_file The source file which contains the error
 * @param line The line number where the error occurred
 * @param column The column number where the error occurred
 * @param text_length The length of the token which caused the error
 * @param msg The error message to display
 * @param include_chain A stack of include files which led to the error
 * @param is_warning Whether the message is a warning or an error
 */
void print_syntax_error_or_warning(
	const std::string& source_file,
	uint32_t line,
	uint32_t column,
	uint32_t text_length,
	const std::string& msg,
	const std::vector<std::string>& include_chain,
	std::shared_ptr<bpp::bpp_program> program,
	bool lsp_mode,
	bool is_warning = false);

void print_parser_errors(
	const std::vector<AST::ParserError>& errors,
	const std::string& source_file,
	const std::vector<std::string>& include_chain,
	std::shared_ptr<bpp::bpp_program> program,
	bool lsp_mode
);

class ErrorOrWarning : public std::runtime_error {
	protected:
		std::string source_file;
		uint32_t line;
		uint32_t column;
		uint32_t text_length;
		std::vector<std::string> include_chain;
		std::shared_ptr<bpp::bpp_program> program;
		bool lsp_mode;
		bool is_warning;

		template <bpp::detail::ErrorReportableListener Listener, bpp::detail::ASTNodePtrORToken T>
		void set_from_listener(Listener* listener, T error_ctx) {
			source_file = listener->get_source_file();
			include_chain = listener->get_include_stack();
			program = listener->get_program();
			lsp_mode = listener->get_lsp_mode();

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
				text_length = static_cast<uint32_t>(error_ctx.getValue().length());
			} else if constexpr(bpp::detail::ASTParameterToken<T>) {
				// Special case: Error reporting on a declared method parameter
				// TODO(@rail5): Kind of hacky to handle special cases. Would prefer a general solution.
				line = error_ctx.getLine();
				column = error_ctx.getCharPositionInLine();
				auto param = error_ctx.getValue();
				text_length = 0;
				if (param.type.has_value()) {
					text_length += 1 + static_cast<uint32_t>(param.type.value().getValue().length()); // '@Type'
					if (param.pointer) text_length += 1; // '*'
					text_length += 1 + static_cast<uint32_t>(param.name.getValue().length()); // ' Name'
				} else {
					text_length = static_cast<uint32_t>(param.name.getValue().length()); // 'Name'
				}
			}
		}
	public:
		ErrorOrWarning() = delete;
		explicit ErrorOrWarning(const std::string& msg) = delete;

		template <bpp::detail::ErrorReportableListener Listener, bpp::detail::ASTNodePtrORToken T>
		inline ErrorOrWarning(Listener* listener, const T& error_ctx, const std::string& msg) : std::runtime_error(msg) {
			set_from_listener(listener, error_ctx);
		}
		
		inline void print() const {
			print_syntax_error_or_warning(
				source_file,
				line,
				column,
				text_length,
				this->what(),
				include_chain,
				program,
				lsp_mode,
				is_warning
			);
		}
};

/**
 * @class SyntaxError
 * @brief An exception thrown when a syntax error is encountered
 * This exception can be constructed from any listener that satisfies the ErrorReportableListener concept,
 * along with an AST node or token that satisfies the ASTNodePtrORToken concept.
 * When thrown, the exception can be caught and printed to display a formatted syntax error message.
 * 
 */
class SyntaxError : public ErrorOrWarning {
	public:
		SyntaxError() = delete;
		explicit SyntaxError(const std::string& msg) = delete;

		template <bpp::detail::ErrorReportableListener Listener, bpp::detail::ASTNodePtrORToken T>
		inline SyntaxError(Listener* listener, const T& error_ctx, const std::string& msg) : ErrorOrWarning(listener, error_ctx, msg) {
			is_warning = false;
		}
};

/**
 * @class Warning
 * @brief A compiler warning that is not fatal to compilation
 * This type should never be thrown. Instead, it should be constructed and displayed via the print() method.
 * Throwing it would halt compilation, which is not desired for warnings.
 * This class likewise can be constructed from any listener that satisfies the ErrorReportableListener concept,
 * along with an AST node or token that satisfies the ASTNodePtrORToken concept.
 * 
 */
class Warning : public ErrorOrWarning {
	public:
		Warning() = delete;
		explicit Warning(const std::string& msg) = delete;

		template <bpp::detail::ErrorReportableListener Listener, bpp::detail::ASTNodePtrORToken T>
		inline Warning(Listener* listener, const T& error_ctx, const std::string& msg) : ErrorOrWarning(listener, error_ctx, msg) {
			is_warning = true;
		}
};

// Helper functions
// Should probably be moved to a separate file or somehow better organized
std::string utf8_substr(const std::string& str, uint32_t start, uint32_t length);
uint32_t utf8_length(const std::string& str);
std::string equal_width_padding(const std::string& str, char padding_char = ' ');

} // namespace ErrorHandling
} // namespace bpp
