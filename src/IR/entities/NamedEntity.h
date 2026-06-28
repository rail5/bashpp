/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>

#include <IR/entities/Entity.h>

namespace bpp::IR {

/**
 * @brief A base class for entities which have names.
 *
 * Not all entities have names, but those that do (e.g., classes, methods, objects) inherit from this class *as well as* from Entity.
 */
class NamedEntity {
	protected:
		std::string name;
	public:
		const std::string& get_name() const { return name; }
		void set_name(const std::string& name) { this->name = name; }
};

} // namespace bpp::IR
