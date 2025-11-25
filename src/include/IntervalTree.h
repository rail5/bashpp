/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <cassert>

template <class T>
class IntervalNode {
	private:
		std::unique_ptr<IntervalNode<T>> _left;
		std::unique_ptr<IntervalNode<T>> _right;
		uint64_t _low;
		uint64_t _high;
		uint64_t _max;
		T _payload;
	public:
		IntervalNode(uint64_t low, uint64_t high, T payload)
			: _low(low), _high(high), _max(high), _payload(payload) {}

		IntervalNode(uint64_t low, uint64_t high) : _low(low), _high(high), _max(high) {}

		uint64_t low() const {
			return _low;
		}
		uint64_t high() const {
			return _high;
		}
		uint64_t max() const {
			return _max;
		}
		T payload() const {
			return _payload;
		}

		std::unique_ptr<IntervalNode<T>>& left() {
			return _left;
		}
		std::unique_ptr<IntervalNode<T>>& right() {
			return _right;
		}

		void set_left(std::unique_ptr<IntervalNode<T>> left) {
			_left = std::move(left);
		}
		void set_right(std::unique_ptr<IntervalNode<T>> right) {
			_right = std::move(right);
		}
		void set_low(uint64_t low) {
			_low = low;
		}
		void set_high(uint64_t high) {
			_high = high;
		}
		void set_max(uint64_t max) {
			_max = max;
		}
		void set_payload(T payload) {
			_payload = payload;
		}
};

/**
 * @class IntervalTree
 *
 * @brief A specialized implementation of an Interval Tree for Bash++'s particular use case.
 *
 * This implementation is designed to efficiently manage and query intervals, allowing for fast insertion,
 * deletion, and overlap detection.
 *
 * Some particularities about our implementation:
 *   1. Partial overlaps are not possible in the Bash++ compiler
 *      All intervals are either entirely contained within wider intervals (e.g., [0 [50, 75] 100])
 *      Or entirely disjoint (e.g., [0, 100] [101, 110])
 *      Partial overlaps can never happen
 *
 *   2. This implementation guarantees that wider (parent) intervals are always placed above narrower (children) intervals in the tree.
 *      If [50, 75] is inserted, and then [0, 100] is inserted afterwards,
 *      The tree will re-order itself such that [0, 100] becomes the new root, with [50, 75] as its child.
 *
 * These invariants are crucial for the tree's primary function in the compiler: find_innermost_overlap
 *
 * @tparam T The type of the payload stored in each interval node.
 */
template <class T>
class IntervalTree {
	private:
		std::unique_ptr<IntervalNode<T>> _root;

		uint64_t find_max(std::unique_ptr<IntervalNode<T>>& node) {
			if (!node) return 0;
			uint64_t left_max = find_max(node->left());
			uint64_t right_max = find_max(node->right());
			return std::max({node->high(), left_max, right_max});
		}

		void insert_node(std::unique_ptr<IntervalNode<T>>& node, uint64_t low, uint64_t high, T payload) {
			if (!node) {
				node = std::make_unique<IntervalNode<T>>(low, high, payload);
				return;
			}

			// ASSERT: Partial overlaps are banned
			bool overlap = (low < node->high() && high > node->low());
			bool child_is_entirely_contained_by_parent = (low >= node->low() && high <= node->high());
			bool parent_is_entirely_contained_by_child = (node->low() >= low && node->high() <= high);

			bool partial_overlap = overlap &&
				!child_is_entirely_contained_by_parent &&
				!parent_is_entirely_contained_by_child;

			assert(!partial_overlap && "Partial overlap given: this should be impossible");

			// If this node should be the parent, we need to insert it before the child
			if (parent_is_entirely_contained_by_child) {
				std::unique_ptr<IntervalNode<T>> old_parent = std::move(node);
				node = std::make_unique<IntervalNode<T>>(low, high, payload);
				node->set_right(std::move(old_parent));
				node->set_max(find_max(node));
				return;
			}

			if (low < node->low()) {
				insert_node(node->left(), low, high, payload);
			} else {
				insert_node(node->right(), low, high, payload);
			}

			// Update the max value
			node->set_max(find_max(node));
		}

		void find_overlaps(std::unique_ptr<IntervalNode<T>>& node, uint64_t point, std::vector<T>& overlaps) {
			if (!node) return;

			// If the current interval overlaps with the point, add it to the overlaps
			if (node->low() <= point && point <= node->high()) {
				overlaps.push_back(node->payload());
			}

			// If the left child has the potential to overlap, search it
			if (node->left() && node->left()->max() >= point) {
				find_overlaps(node->left(), point, overlaps);
			}

			// If the right child has the potential to overlap, search it
			if (node->right() && node->right()->low() <= point) {
				find_overlaps(node->right(), point, overlaps);
			}
		}

	public:
		IntervalTree() : _root(nullptr) {}
		void insert(uint64_t low, uint64_t high, T payload) {
			if (_root == nullptr) {
				_root = std::make_unique<IntervalNode<T>>(low, high, payload);
				return;
			}

			insert_node(_root, low, high, payload);
		}

		std::vector<T> find_overlaps(uint64_t point) {
			std::vector<T> overlaps;
			find_overlaps(_root, point, overlaps);
			return overlaps;
		}

		/**
		 * @brief Find the innermost interval that contains the given point.
		 *
		 * This function traverses the interval tree to find the innermost interval
		 * that contains the specified point.
		 *
		 * Given our tree's invariants, this task is equivalent to either of:
		 *    1. Finding the overlapping interval with the highest start position.
		 *    2. Finding the deepest interval in the tree that contains the point.
		 *
		 * So, this implementation simply traverses the tree, and returns the *last* overlapping node that it finds,
		 * which is guaranteed to be the innermost one due to the tree's structure.
		 *
		 * @param point The point to check for overlaps.
		 * @return T The payload of the innermost overlapping interval, or a default-constructed T if none is found.
		 */
		T find_innermost_overlap(uint64_t point) {
			std::vector<T> overlaps = find_overlaps(point);
			if (overlaps.empty()) {
				return T();
			}
			// Return the overlap with the highest start position
			// Given our tree's invariants, this is equivalent to the .back() of the vector
			return overlaps.back();
		}
};
