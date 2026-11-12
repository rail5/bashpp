/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>

#include <filesystem>
#include <optional>

namespace bpp::IR {

class IncludeStatement : public CodeEntity {
	private:
		/// @brief The path to the included source file
		std::filesystem::path included_file_path;

		/**
		 * @brief Only used for dynamic includes.
		 * The runtime path to the compiled version of the included file,
		 * if specified by the user with an `as` clause in the include statement.
		 * E.g., `@include dynamic "somefile.bpp" as "/path/to/compiled-script.sh"`
		 *
		 * If no "as" path is given, the runtime path is assumed to be the same as the source path,
		 * with the extension replaced by `.sh` (e.g., `somefile.bpp` -> `somefile.sh`, or `somefile` (no extension) -> `somefile.sh`).
		 */
		std::optional<std::filesystem::path> as_path;

		bool is_dynamic_include = false;

		/**
		 * @brief If false, no code will be generated from this include statement or its child entities.
		 * This is used when `@include_always` is not specified, and the included file has already been included in the current program.
		 * Since v0.9.0, 'include' is once-only by default.
		 */
		bool codegen_enabled = true;

		std::filesystem::path getRuntimePath() const;
	public:
		IncludeStatement() = delete;
		IncludeStatement(const std::filesystem::path& included_file_path, bool is_dynamic_include)
			: included_file_path(included_file_path), is_dynamic_include(is_dynamic_include) {}

		const std::filesystem::path& getIncludedFilePath() const { return included_file_path; }
		bool isDynamicInclude() const { return is_dynamic_include; }

		void setAsPath(const std::filesystem::path& path) { as_path = path; }
		void disableCodegen() { codegen_enabled = false; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
};

} // namespace bpp::IR
