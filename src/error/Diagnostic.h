/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <filesystem>
#include <vector>
#include <optional>
#include <cstdint>

#include <error/detail.h>

namespace bpp::IR {
class SourceIndex;
} // namespace bpp::IR

namespace bpp::ErrorHandling {

enum class DiagnosticType : std::uint8_t {
	DIAGNOSTIC_ERROR,
	DIAGNOSTIC_WARNING,
	DIAGNOSTIC_INFO,
	DIAGNOSTIC_HINT,
};

class Diagnostic : public std::runtime_error {
	public:
		Diagnostic() = delete;
		Diagnostic(
			const std::string& error_message,
			const std::vector<std::filesystem::path>& include_chain,
			std::uint32_t line, std::uint32_t column, std::uint32_t text_length,
			DiagnosticType type,
			const std::optional<std::string>& warning_cli_string = std::nullopt
		) : std::runtime_error(error_message),
			include_chain(include_chain),
			line(line), column(column), error_text_length(text_length),
			type(type),
			warning_cli_string(warning_cli_string)
		{}

		Diagnostic(
			const std::string& error_message,
			const std::vector<std::filesystem::path>& include_chain,
			std::uint32_t line, std::uint32_t column, std::uint32_t text_length,
			DiagnosticType type,
			const std::optional<std::string_view>& warning_cli_string
		) : std::runtime_error(error_message),
			include_chain(include_chain),
			line(line), column(column), error_text_length(text_length),
			type(type)
		{
			if (warning_cli_string) this->warning_cli_string = std::string(*warning_cli_string);
		}

		template <bpp::detail::ASTNodePtrORToken T>
		Diagnostic(
			const std::string& error_message,
			const std::vector<std::filesystem::path>& include_chain,
			const T& error_ctx,
			DiagnosticType type,
			const std::optional<std::string>& warning_cli_string = std::nullopt
		) : std::runtime_error(error_message),
			include_chain(include_chain),
			type(type),
			warning_cli_string(warning_cli_string)
		{
			if constexpr (bpp::detail::ASTNodePtrType<T>) {
				line = error_ctx->getPosition().line;
				column = error_ctx->getPosition().column;
				if (error_ctx->getEndPosition().line == error_ctx->getPosition().line) {
					error_text_length = error_ctx->getEndPosition().column - error_ctx->getPosition().column;
				} else {
					error_text_length = UINT32_MAX; // Highlight the entire line if the error spans multiple lines
				}
			} else if constexpr (bpp::detail::ASTStringToken<T>) {
				line = error_ctx.getLine();
				column = error_ctx.getColumn();
				error_text_length = static_cast<std::uint32_t>(error_ctx.getValue().length());
			} else if constexpr (bpp::detail::ASTParameterToken<T>) {
				// TODO(@rail5): Hack. Special-casing method parameters instead of a general solution.
				line = error_ctx.getLine();
				column = error_ctx.getColumn();
				auto param = error_ctx.getValue();
				error_text_length = 0;
				if (param.type.has_value()) {
					error_text_length += 1 + static_cast<std::uint32_t>(param.type.value().getValue().length()); // '@Type'
					if (param.pointer) error_text_length += 1; // '*'
					error_text_length += 1; // ' '
				}
				error_text_length += static_cast<std::uint32_t>(param.name.getValue().length()); // 'Name'
			}
		}

		template <bpp::detail::ASTNodePtrORToken T>
		Diagnostic(
			const std::string& error_message,
			const std::vector<std::filesystem::path>& include_chain,
			const T& error_ctx,
			DiagnosticType type,
			const std::optional<std::string_view>& warning_cli_string
		) : Diagnostic(error_message, include_chain, error_ctx, type, std::optional<std::string>{})
		{
			if (warning_cli_string) this->warning_cli_string = std::string(*warning_cli_string);
		}

		void print_to_stderr() const;

		const std::vector<std::filesystem::path>& getIncludeChain() const { return include_chain; }
		std::uint32_t getLine() const { return line; }
		std::uint32_t getColumn() const { return column; }
		std::uint32_t getTextLength() const { return error_text_length; }
		DiagnosticType getType() const { return type; }
		const std::optional<std::string>& getWarningCliString() const { return warning_cli_string; }

		void setIncludeChain(const std::vector<std::filesystem::path>& include_chain) { this->include_chain = include_chain; }
		void setLine(std::uint32_t line) { this->line = line; }
		void setColumn(std::uint32_t column) { this->column = column; }
		void setTextLength(std::uint32_t text_length) { this->error_text_length = text_length; }
		void setType(DiagnosticType type) { this->type = type; }
		void setWarningCliString(const std::optional<std::string>& warning_cli_string) { this->warning_cli_string = warning_cli_string; }
	private:
		std::vector<std::filesystem::path> include_chain;
		std::uint32_t line = 0;
		std::uint32_t column = 0;
		std::uint32_t error_text_length = 1;
		DiagnosticType type = DiagnosticType::DIAGNOSTIC_ERROR;
		std::optional<std::string> warning_cli_string = std::nullopt;
};

} // namespace bpp::ErrorHandling
