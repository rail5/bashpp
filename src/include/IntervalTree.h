/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>

/**
 * @class FlatIntervalTree
 * @brief A specialized implementation of an Interval Tree optimized for Bash++'s particular use case.
 *
 * This implementation is designed to efficiently store and query intervals representing
 * code entities in Bash++ source files. It supports insertion of intervals and querying
 * for the innermost overlapping interval at a given point.
 *
 *
 * It's possible to design this as a "flat" structure because of our invariants:
 *
 * 1. Intervals are only inserted and never removed.
 *
 * 2. Queries are only made after all insertions are complete.
 *
 * 3. Intervals do not partially overlap; they are either entirely nested or entirely disjoint.
 *
 *
 * These invariants allow us to use a sorted vector and binary search for efficient querying,
 * avoiding the complexity of a traditional tree structure.
 * 
 * @tparam T The type of the payload associated with each interval.
 */
template <class T>
class FlatIntervalTree {
private:
	struct Interval {
		uint64_t low, high;
		T payload;
		
		bool contains(uint64_t point) const { return low <= point && point <= high; }
		bool contains(const Interval& other) const { return low <= other.low && high >= other.high; }
	};
	
	std::vector<Interval> intervals;
	bool sorted = false;
	
	void ensure_sorted() {
		if (!sorted) {
			// Sort by low, then by high (descending) so wider intervals come first
			std::sort(intervals.begin(), intervals.end(),
				[](const Interval& a, const Interval& b) {
					return a.low < b.low || (a.low == b.low && a.high > b.high);
				});
			sorted = true;
		}
	}
	
public:
	void insert(uint64_t low, uint64_t high, T payload) {
		// TODO(@rail5): Assertions to ensure that our invariants are maintained
		intervals.push_back({low, high, payload});
		sorted = false;
	}
	
	/**
	 * @brief Find the innermost interval that overlaps a given point.
	 *
	 * This function performs a binary search to efficiently locate the innermost interval
	 * that contains the specified point. If no such interval exists, it returns a default-constructed T.
	 * 
	 * @param point The point to query.
	 * @return T The payload of the innermost overlapping interval, or default-constructed T if none found.
	 */
	T find_innermost_overlap(uint64_t point) {
		ensure_sorted();
		
		// Binary search for first interval with low <= point
		auto it = std::upper_bound(intervals.begin(), intervals.end(), point,
			[](uint64_t point, const Interval& interval) {
				return point < interval.low;
			});
		
		if (it == intervals.begin()) return T();
		
		// Scan backwards to find innermost (since sorted by low then wide-first)
		--it;
		const Interval* best = nullptr;
		for (; it >= intervals.begin() && it->low <= point; --it) {
			if (it->contains(point)) {
				if (!best || it->low > best->low) {
					best = &*it;
				}
			}

			if (it == intervals.begin()) break; // Prevent underflow/UB
		}
		return best ? best->payload : T();
	}
};
