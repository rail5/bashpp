/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <filesystem>
#include <cstdint>

namespace bpp::IR {

/**
 * @struct FilePosition
 * @brief Represents a position in a source file by line and column
 *
 * This struct is used as the key type in the EntityMap to represent positions in source files.
 * It is represented as a single unsigned 64-bit integer for efficient storage and comparison.
 * The high 32 bits represent the line number, and the low 32 bits represent the column number.
 * 
 */
struct FilePosition {
	std::uint32_t line = 0;
	std::uint32_t column = 0;

	operator std::uint64_t() const {
		// Encode line and column into a single std::uint64_t
		// The first 32 bits represent the line number,
		// and the last 32 bits represent the column number.
		// This permits perfect sorting of file positions
		return (static_cast<std::uint64_t>(line) << 32) | column;
	}

	FilePosition() = default;
	FilePosition(std::uint32_t line, std::uint32_t column)
		: line(line), column(column) {}
};

struct SourceLocation {
	std::filesystem::path source_file;
	FilePosition position;

	SourceLocation() = default;
	SourceLocation(const std::filesystem::path& source_file, FilePosition position)
		: source_file(source_file), position(position) {}
	SourceLocation(const std::filesystem::path& source_file, std::uint32_t line, std::uint32_t column)
		: source_file(source_file), position(line, column) {}
};

struct SourceLocationRange {
	std::filesystem::path source_file;
	FilePosition start;
	FilePosition end;

	SourceLocationRange() = default;
	SourceLocationRange(const std::filesystem::path& source_file, FilePosition start, FilePosition end)
		: source_file(source_file), start(start), end(end) {}
	SourceLocationRange(const SourceLocation& start, const SourceLocation& end)
		: source_file(start.source_file), start(start.position), end(end.position) {}
	SourceLocationRange(const std::filesystem::path& source_file, std::uint32_t start_line, std::uint32_t start_column, std::uint32_t end_line, std::uint32_t end_column)
		: source_file(source_file), start(start_line, start_column), end(end_line, end_column) {}
};

} // namespace bpp::IR
