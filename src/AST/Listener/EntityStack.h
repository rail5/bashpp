/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <vector>

#include <IR/bpp.h>
#include <IR/entities/Entity.h>

namespace bpp::IR {

/**
 * @brief A stack of entities used during AST traversal to keep track of the current context.
 */
class EntityStack {
	public:
		/**
		 * @brief Push a new entity onto the stack.
		 *
		 * @param entity The entity to push onto the stack.
		 */
		void push(std::unique_ptr<Entity> entity);

		/**
		 * @brief Pop the top entity off the stack and return it.
		 *
		 * @return std::unique_ptr<Entity> The entity that was popped off the stack.
		 */
		std::unique_ptr<Entity> pop();

		/**
		 * @brief Pop the top entity off the stack and return it as a specific type.
		 * This performs a simple static_cast. No type checking is performed; that is the caller's responsibility.
		 *
		 * @tparam T The type to cast the popped entity to.
		 * @return std::unique_ptr<T> The entity that was popped off the stack, cast to type T.
		 */
		template <class T>
		std::unique_ptr<T> pop_as() {
			auto entity = std::move(stack.back());
			stack.pop_back();
			return std::unique_ptr<T>(static_cast<T*>(entity.release()));
		}

		/**
		 * @brief View the top entity on the stack without popping it.
		 * 
		 * @return Entity* A non-owning pointer to the top entity on the stack
		 */
		Entity* top() const;

		/**
		 * @brief View the top entity on the stack without popping it, cast to a specific type.
		 * This performs a simple static_cast. No type checking is performed; that is the caller's responsibility.
		 * @tparam T The type to cast the top entity to.
		 * @return T* A non-owning pointer to the top entity on the stack, cast to type T
		 */
		template <class T>
		T* top_as() const { return static_cast<T*>(top()); }

		bool empty() const { return stack.empty(); }
		std::size_t size() const { return stack.size(); }

		auto begin() const { return stack.begin(); }
		auto end() const { return stack.end(); }
		auto rbegin() const { return stack.rbegin(); }
		auto rend() const { return stack.rend(); }
	private:
		std::vector<std::unique_ptr<Entity>> stack;
};

} // namespace bpp::IR
