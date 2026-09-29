/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "EntityStack.h"

#include <IR/entities/Entity.h>

namespace bpp::IR {

void EntityStack::push(std::unique_ptr<Entity> entity) { stack.push_back(std::move(entity)); }

std::unique_ptr<Entity> EntityStack::pop() {
	auto entity = std::move(stack.back());
	stack.pop_back();
	return entity;
}

Entity* EntityStack::top() const {
	return stack.back().get();
}

} // namespace bpp::IR
