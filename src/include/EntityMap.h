/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include "IntervalTree.h"

#include <vector>

// Forward decl.
namespace bpp::IR {
class Entity;
} // namespace bpp::IR

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
 * Invariants:
 *  - Any two intervals in the map are either strictly nested or strictly disjoint.
 *    I.e.:
 *     1. [5, 10] and [6, 8] can coexist -- the second interval is strictly nested within the first.
 *     2. [5, 10] and [11, 15] can coexist -- the two intervals are strictly disjoint.
 *     3. [5, 10] and [8, 12] cannot coexist -- the two intervals partially overlap, which is not allowed.
 *     4. [5, 10] and [5, 10] cannot coexist -- the two intervals are identical, which is not allowed.
 * - Intervals are only inserted and never removed.
 * - Queries are only made after all insertions are complete.
 */
class EntityMap {
	private:
		FlatIntervalTree<const bpp::IR::Entity*> tree;
	public:
		/**
		 * @brief Add an entity to the entity map
		 */
		void insert(FilePosition start, FilePosition end, const bpp::IR::Entity* entity) {
			tree.insert(start, end, entity);
		}

		const bpp::IR::Entity* find(FilePosition point) const {
			return tree.find_innermost_overlap(point);
		}

		/**
		 * @brief Find the active code entity at a specific line and column
		 */
		const bpp::IR::Entity* find(std::uint32_t line, std::uint32_t column) const {
			return find(FilePosition(line, column));
		}

		void updateEntityAddress(const bpp::IR::Entity* old_address, const bpp::IR::Entity* new_address) {
			tree.updatePayload(
				[old_address](const bpp::IR::Entity* payload) { return payload == old_address; },
				[new_address](const bpp::IR::Entity*) { return new_address; }
			);
		}
};

/**
 * @brief A map of file positions to multiple Bash++ entities
 * 
 * We have one slightly different invariant compared to the ordinary EntityMap.
 * In the ordinary EntityMap, the position of any two entities must be strictly nested or strictly disjoint.
 * In this map, the position of two entities is also permitted to be identical, in which case both entities will be returned for that position.
 */
class MultipleEntityMap {
	private:
		FlatIntervalTree<std::vector<const bpp::IR::Entity*>> tree;
	public:
		void insert(FilePosition start, FilePosition end, const bpp::IR::Entity* entity) {
			if (auto* existing = tree.find(start, end)) {
				existing->payload.push_back(entity);
			} else {
				tree.insert(start, end, std::vector<const bpp::IR::Entity*>{entity});
			}
		}

		std::vector<const bpp::IR::Entity*> find(FilePosition point) const {
			return tree.find_innermost_overlap(point);
		}

		std::vector<const bpp::IR::Entity*> find(std::uint32_t line, std::uint32_t column) const {
			return find(FilePosition(line, column));
		}

		void updateEntityAddress(const bpp::IR::Entity* old_address, const bpp::IR::Entity* new_address) {
			tree.updatePayload(
				[old_address](const std::vector<const bpp::IR::Entity*>& payload) {
					return std::ranges::contains(payload, old_address);
				},
				[old_address, new_address](const std::vector<const bpp::IR::Entity*>& payload) {
					std::vector<const bpp::IR::Entity*> new_payload = payload;
					std::replace(new_payload.begin(), new_payload.end(), old_address, new_address);
					return new_payload;
				}
			);
		}
};
