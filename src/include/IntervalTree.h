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

	// Getters
	uint64_t low() const { return _low; }
	uint64_t high() const { return _high; }
	uint64_t max() const { return _max; }
	const T& payload() const { return _payload; }
	
	std::unique_ptr<IntervalNode<T>>& left() { return _left; }
	std::unique_ptr<IntervalNode<T>>& right() { return _right; }

	void set_left(std::unique_ptr<IntervalNode<T>> left) {
		_left = std::move(left);
	}
	void set_right(std::unique_ptr<IntervalNode<T>> right) {
		_right = std::move(right);
	}
	void set_max(uint64_t max) { _max = max; }
	void set_payload(T payload) { _payload = payload; }
	
	// Update max based on children (non-recursive)
	void update_max() {
		_max = _high;
		if (_left && _left->_max > _max) _max = _left->_max;
		if (_right && _right->_max > _max) _max = _right->_max;
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

	// Fast, non-recursive max computation for a node's subtree
	static uint64_t compute_subtree_max(const IntervalNode<T>* node) {
		if (!node) return 0;
		
		uint64_t max_val = node->high();
		if (node->left()) {
			max_val = std::max(max_val, node->left()->max());
		}
		if (node->right()) {
			max_val = std::max(max_val, node->right()->max());
		}
		return max_val;
	}
public:
	IntervalTree() : _root(nullptr) {}
	
	/**
	 * @brief Inserts a new interval node into the tree, maintaining the tree's invariants.
	 * 
	 * @param low The lower bound of the interval to insert.
	 * @param high The upper bound of the interval to insert.
	 * @param payload The payload associated with the interval.
	 */
	void insert(uint64_t low, uint64_t high, T payload) {
		if (!_root) {
			_root = std::make_unique<IntervalNode<T>>(low, high, payload);
			return;
		}
		
		// Store the path for backtracking and max updates
		struct InsertPath {
			std::unique_ptr<IntervalNode<T>>* ptr;  // Pointer to the unique_ptr in parent
			IntervalNode<T>* node;  // Current node
		};
		
		std::vector<InsertPath> path;
		path.reserve(32);  // Pre-allocate reasonable depth
		
		// Find insertion point
		std::unique_ptr<IntervalNode<T>>* current = &_root;
		IntervalNode<T>* node = current->get();
		
		while (node) {
			// Check for containment
			bool child_in_parent = (low >= node->low() && high <= node->high());
			bool parent_in_child = (node->low() >= low && node->high() <= high);
			bool overlap = (low < node->high() && high > node->low());
			bool partial_overlap = overlap && !child_in_parent && !parent_in_child;
			
			assert(!partial_overlap && "Partial overlap given: this should be impossible");
			
			// Handle promotion case
			if (parent_in_child) {
				auto new_node = std::make_unique<IntervalNode<T>>(low, high, payload);
				new_node->set_right(std::move(*current));
				new_node->update_max();
				*current = std::move(new_node);
				return;  // No need to update ancestors since we replaced the node
			}
			
			// Record path for backtracking
			path.push_back({current, node});
			
			// Move to appropriate child
			if (low < node->low()) {
				current = &(node->left());
			} else {
				current = &(node->right());
			}
			node = current->get();
		}
		
		// Insert new node at leaf position
		*current = std::make_unique<IntervalNode<T>>(low, high, payload);
		
		// Update max values back up the path
		for (auto it = path.rbegin(); it != path.rend(); ++it) {
			it->node->update_max();
		}
	}

	/**
	 * @brief Finds the innermost interval that overlaps with the given point.
	 * 
	 * @param point The point to check for overlap.
	 * @return T The payload of the innermost overlapping interval, or a default-constructed T if none found.
	 */
	T find_innermost_overlap(uint64_t point) const {
		if (!_root) return T();
		
		IntervalNode<T>* current = _root.get();
		IntervalNode<T>* best = nullptr;
		uint64_t best_start = 0;
		
		struct StackFrame {
			IntervalNode<T>* node;
			bool visited_left;
		};
		
		std::vector<StackFrame> stack;
		stack.reserve(64);
		stack.push_back({current, false});
		
		while (!stack.empty()) {
			auto& frame = stack.back();
			
			if (!frame.visited_left) {
				// Check if we should visit left subtree
				if (frame.node->left() && frame.node->left()->max() >= point) {
					stack.push_back({frame.node->left().get(), false});
					frame.visited_left = true;
					continue;
				}
				frame.visited_left = true;
			}
			
			// Process current node
			if (frame.node->low() <= point && point <= frame.node->high()) {
				if (!best || frame.node->low() > best_start) {
					best = frame.node;
					best_start = frame.node->low();
				}
			}
			
			// Check if we should visit right subtree
			if (frame.node->right() && frame.node->right()->low() <= point) {
				// Replace current frame with right child
				frame = {frame.node->right().get(), false};
				continue;
			}
			
			// Pop and continue
			stack.pop_back();
		}
		
		return best ? best->payload() : T();
	}
};
