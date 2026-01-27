/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once
#include <optional>
#include <memory>
#include "IntervalTree.h"


// Forward decl.
namespace bpp {
class bpp_entity;
}

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
	uint32_t line;
	uint32_t column;

	operator uint64_t() const {
		// Encode line and column into a single uint64_t
		// The first 32 bits represent the line number,
		// and the last 32 bits represent the column number.
		// This permits perfect sorting of file positions
		return (static_cast<uint64_t>(line) << 32) | column;
	}

	FilePosition(uint32_t line, uint32_t column)
		: line(line), column(column) {}
};

/**
 * @class EntityMap
 * @brief A map of file positions to Bash++ container entities
 *
 * The EntityMap is used to mark regions of source files with the entities that are active in those regions.
 * For instance, if a class is defined from line 10 to line 20, the EntityMap will map that range to the class entity.
 * A method internal to that class defined from line 12 to line 15 will map that range to the method entity.
 *
 * This allows us to quickly look up the active entity at any given position in the source file.
 * This class is used primarily by the language server to resolve entities for features like go-to-definition and rename refactoring.
 * 
 */
class EntityMap {
	private:
		FlatIntervalTree<std::shared_ptr<bpp::bpp_entity>> tree;
	public:

		/**
		 * @brief Add an entity to the entity map
		 */
		void insert(FilePosition start, FilePosition end, const std::shared_ptr<bpp::bpp_entity>& entity) {
			tree.insert(start, end, entity);
		}

		std::shared_ptr<bpp::bpp_entity> find(FilePosition point) {
			return tree.find_innermost_overlap(point);
		}

		/**
		 * @brief Find the active code entity at a specific line and column
		 */
		std::shared_ptr<bpp::bpp_entity> find(uint32_t line, uint32_t column) {
			return find(FilePosition(line, column));
		}
};
