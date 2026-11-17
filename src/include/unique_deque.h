/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <concepts>
#include <deque>
#include <algorithm>

/**
 * @brief A deque that preserves element uniqueness, i.e., it does not allow duplicate elements.
 * 
 * @tparam T The type of elements stored in the deque. Must be comparable with operator==.
 */
template <class T>
requires std::equality_comparable<T>
class unique_deque {
	private:
		std::deque<T> data;
	public:
		unique_deque() = default;
		unique_deque(const unique_deque& other) = default;
		unique_deque& operator=(const unique_deque& other) = default;
		unique_deque(unique_deque&& other) noexcept = default;
		unique_deque& operator=(unique_deque&& other) noexcept = default;
		~unique_deque() = default;

		unique_deque(std::initializer_list<T> init) {
			for (const auto& value : init) push_back(value);
		}

		void push_back(const T& value) {
			if (std::find(data.begin(), data.end(), value) == data.end()) {
				data.push_back(value);
			}
		}

		void push_front(const T& value) {
			if (std::find(data.begin(), data.end(), value) == data.end()) {
				data.push_front(value);
			}
		}

		void pop_back() { data.pop_back(); }
		void pop_front() { data.pop_front(); }

		const T& front() const { return data.front(); }
		const T& back() const { return data.back(); }

		bool empty() const { return data.empty(); }
		std::size_t size() const { return data.size(); }
		void clear() { data.clear(); }

		// Iterators
		auto begin() { return data.begin(); }
		auto end() { return data.end(); }
		auto begin() const { return data.begin(); }
		auto end() const { return data.end(); }
};
