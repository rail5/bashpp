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
		IntervalTree<std::shared_ptr<bpp::bpp_entity>> tree;
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
