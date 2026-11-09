/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <filesystem>
#include <ranges>
#include <span>
#include <unordered_map>
#include <vector>
#include <deque>
#include <optional>

#include <IR/bpp.h>
#include <include/EntityMap.h>
#include <error/Diagnostic.h>

namespace bpp::IR {

struct SourceLocation {
	std::filesystem::path source_file;
	FilePosition position;

	SourceLocation() = delete;
	SourceLocation(const std::filesystem::path& source_file, FilePosition position)
		: source_file(source_file), position(position) {}
	SourceLocation(const std::filesystem::path& source_file, std::uint32_t line, std::uint32_t column)
		: source_file(source_file), position(line, column) {}
};

struct SourceLocationRange {
	std::filesystem::path source_file;
	FilePosition start;
	FilePosition end;

	SourceLocationRange() = delete;
	SourceLocationRange(const std::filesystem::path& source_file, FilePosition start, FilePosition end)
		: source_file(source_file), start(start), end(end) {}
	SourceLocationRange(const SourceLocation& start, const SourceLocation& end)
		: source_file(start.source_file), start(start.position), end(end.position) {}
	SourceLocationRange(const std::filesystem::path& source_file, std::uint32_t start_line, std::uint32_t start_column, std::uint32_t end_line, std::uint32_t end_column)
		: source_file(source_file), start(start_line, start_column), end(end_line, end_column) {}
};

namespace views {

/// A non-owning view of a std::filesystem::path
class path_view {
	private:
		const std::filesystem::path* path_ptr;
	public:
		path_view() = delete;
		path_view(const std::filesystem::path& path) : path_ptr(&path) {}
		path_view& operator=(const std::filesystem::path& path) {
			path_ptr = &path;
			return *this;
		}
		path_view& operator=(const std::filesystem::path* path) {
			path_ptr = path;
			return *this;
		}
		[[nodiscard]] explicit operator bool() const { return path_ptr != nullptr; }

		[[nodiscard]] const std::filesystem::path& get() const { return *path_ptr; }
		[[nodiscard]] bool operator==(const path_view& other) const {
			return *path_ptr == *other.path_ptr;
		}
		[[nodiscard]] bool operator!=(const path_view& other) const { return !(*this == other); }
		[[nodiscard]] bool operator<(const path_view& other) const {
			return *path_ptr < *other.path_ptr;
		}
		[[nodiscard]] bool operator>(const path_view& other) const {
			return *path_ptr > *other.path_ptr;
		}
		[[nodiscard]] bool operator<=(const path_view& other) const { return !(*this > other); }
		[[nodiscard]] bool operator>=(const path_view& other) const { return !(*this < other); }
		[[nodiscard]] std::string string() const { return path_ptr->string(); }

		[[nodiscard]] bool operator==(const std::filesystem::path& other) const { return *path_ptr == other; }
		[[nodiscard]] bool operator!=(const std::filesystem::path& other) const { return *path_ptr != other; }
		[[nodiscard]] bool operator<(const std::filesystem::path& other) const { return *path_ptr < other; }
		[[nodiscard]] bool operator>(const std::filesystem::path& other) const { return *path_ptr > other; }
		[[nodiscard]] bool operator<=(const std::filesystem::path& other) const { return *path_ptr <= other; }
		[[nodiscard]] bool operator>=(const std::filesystem::path& other) const { return *path_ptr >= other; }

		// Hashing for unordered_map
		struct Hash {
			[[nodiscard]] std::size_t operator()(const path_view& pv) const {
				return std::hash<std::string>()(pv.path_ptr->string());
			}
		};

		explicit operator std::filesystem::path() const { return *path_ptr; }

		friend std::ostream& operator<<(std::ostream& os, const path_view& pv) {
			os << pv.path_ptr->string();
			return os;
		}
};

struct SourceLocationView {
	path_view source_file;
	FilePosition position;

	SourceLocationView() = delete;
	SourceLocationView(path_view source_file, FilePosition position)
		: source_file(source_file), position(position) {}
	SourceLocationView(path_view source_file, std::uint32_t line, std::uint32_t column)
		: source_file(source_file), position(line, column) {}
	SourceLocationView(const SourceLocation& loc)
		: source_file(loc.source_file), position(loc.position) {}

	operator SourceLocation() const { return {source_file.get(), position}; }
};

struct SourceLocationRangeView {
	path_view source_file;
	FilePosition start;
	FilePosition end;

	SourceLocationRangeView() = delete;
	SourceLocationRangeView(path_view source_file, FilePosition start, FilePosition end)
		: source_file(source_file), start(start), end(end) {}
	SourceLocationRangeView(const SourceLocationView& start, const SourceLocationView& end)
		: source_file(start.source_file), start(start.position), end(end.position) {}
	SourceLocationRangeView(path_view source_file, std::uint32_t start_line, std::uint32_t start_column, std::uint32_t end_line, std::uint32_t end_column)
		: source_file(source_file), start(start_line, start_column), end(end_line, end_column) {}
	SourceLocationRangeView(const SourceLocationRange& range)
		: source_file(range.source_file), start(range.start), end(range.end) {}

	operator SourceLocationRange() const { return {source_file.get(), start, end}; }
};

} // namespace views

/**
 * @brief An index of source files: entity references and diagnostics.
 * This structure is used by the language server to provide features like "go to definition" and "find all references."
 *
 * For each source file, we store:
 *  - A "source file map" of code entities (i.e., such-and-such function extends from position X to position Y, etc),
 *  - A source file map of entity references (i.e., object A is referenced by line A, columns B-C, etc),
 *  - A list of diagnostics (errors and warnings) for that source file.
 *
 * For each entity, we store a list of all source locations where that entity is referenced.
 *
 * This allows us to quickly answer questions like:
 *  - "What entity is referenced at this position in this source file?"
 *  - "Where are all the places that this entity is referenced?"
 * As well as getting all diagnostics for a given source file.
 */
class SourceIndex final {
	public:
		void addSourceFile(const std::filesystem::path& source_file) { fetchOrAdd(source_file); }
		void addDiagnostic(const std::filesystem::path& source_file, const bpp::ErrorHandling::Diagnostic& diagnostic);

		/**
		 * @brief Mark that an entity is referenced at a given source location.
		 *
		 * @param entity The entity that is referenced. 
		 * @param range The source location of the reference.
		 * @note In general, @p range.end should be equal to @p range.start + the length of the name of the referenced entity. There are exceptions to this, e.g. 'include' directives reference IncludedProgram entities, which do not have names.
		 */
		void addReference(const Entity* entity, const SourceLocationRange& range);

		void setDefinitionPosition(const Entity* entity, const SourceLocationRange& range);

		/**
		 * @brief Mark an entity as the "context entity" for a given range of positions in a source file.
		 * I.e., this entity is the "container" or "context" for all code within that range:
		 *   If an LSP client requests completions, the language server will only return completions valid within this context.
		 * Containers can be either strictly nested or entirely disjoint, but they cannot partially overlap.
		 *
		 * @param range The range of positions in the source file for which the context entity is defined.
		 * @param entity The entity that is the context for the given range of positions.
		 */
		void markContextEntity(const SourceLocationRange& range, const Entity* entity);

		/// @brief Get all diagnostics for a given source file
		std::span<const bpp::ErrorHandling::Diagnostic> getDiagnostics(const std::filesystem::path& source_file) const;
		std::span<const bpp::ErrorHandling::Diagnostic> getDiagnostics(views::path_view source_file) const;

		/// @brief Get all references to the given entity.
		std::span<const views::SourceLocationRangeView> getReferences(const Entity* entity) const;

		/// @brief Get the definition position of the given entity, if any.
		std::optional<views::SourceLocationRangeView> getDefinitionPosition(const Entity* entity) const;

		/**
		 * @brief Get the entity referenced at the given source location, if any.
		 * 
		 * @param location The source location to look up.
		 * @return const Entity* A pointer to the entity referenced at the given location, or nullptr if no entity is referenced at that location.
		 */
		const Entity* getReferencedEntity(const SourceLocation& location) const;
		const Entity* getReferencedEntity(views::SourceLocationView location) const;

		const Entity* getContextEntity(const SourceLocation& location) const;
		const Entity* getContextEntity(views::SourceLocationView location) const;

		auto getAllSourceFiles() const { return files | std::ranges::views::keys; }

		SourceIndex() = default;
	private:
		/// DO NOT optimize for faster lookup: reallocations done by std::vector &co would invalidate views.
		std::deque<std::filesystem::path> paths;

		views::path_view fetchOrAdd(const std::filesystem::path& path);

		struct SourceFileData {
			EntityMap context_entities;
			EntityMap entity_references;
			std::vector<bpp::ErrorHandling::Diagnostic> diagnostics;
		};
		std::unordered_map<views::path_view, SourceFileData, views::path_view::Hash> files;

		struct EntityData {
			std::vector<views::SourceLocationRangeView> references;
			std::optional<views::SourceLocationRangeView> definition_position;
		};
		std::unordered_map<const Entity*, EntityData> entities;
};

} // namespace bpp::IR
