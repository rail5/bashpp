/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once
#include <optional>
#include "EntityNode.h"

using IntervalTree = lib_interval_tree::interval_tree<IntervalType, EntityHooks>;

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

class EntityMap {
	private:
		IntervalTree tree;
	public:

		/**
		 * @brief Add an entity to the entity map
		 */
		void insert(FilePosition start, FilePosition end, const std::shared_ptr<bpp::bpp_entity>& entity) {
			auto it = tree.insert({start, end});
			it.node()->payload = entity;
		}

		std::shared_ptr<bpp::bpp_entity> find(FilePosition point) {
			// Because of our strictly-nested tree structure,
			// The innermost active entity at a given point
			// Is guaranteed to be the entity which overlaps that point and has the highest start position
			// We can only make this assumption safely because code entities cannot overlap in arbitrary ways
			// They can only overlap in a way such that one entity is fully contained within another
			//
			// Eg, we can only have structures such as [0  [100  [300 350]  500]  1000]
			// Imagine we're looking for the innermost active entity at point 301
			// All of the above entities overlap that point, but the innermost active entity is the one starting at 300
			// Ie, the one with the highest start position
			//
			// It is impossible in our case to have any other kind of overlap, such as [0 100] and [50 150]
			// (Where an inner entity is not *entirely* contained within the outer entity)
			// If that were possible, this method would not work correctly
			std::optional<uint64_t> highestStart = std::nullopt;
			std::shared_ptr<bpp::bpp_entity> innermostEntity;
			tree.overlap_find_all({point, point}, [&](auto const& it) {
				// This is also based on the assumption (valid in our case)
				// That no two entities can start at the same point
				if (!highestStart.has_value() || it->low() > highestStart) {
					highestStart = it->low();
					innermostEntity = it.node()->payload;
				}
				return true; // Continue searching
			});
			return innermostEntity;
		}

		/**
		 * @brief Find the active code entity at a specific line and column
		 */
		std::shared_ptr<bpp::bpp_entity> find(uint32_t line, uint32_t column) {
			return find(FilePosition(line, column));
		}
};
