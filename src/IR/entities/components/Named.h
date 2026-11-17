/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <string_view>

namespace bpp::IR::Components {

/**
 * @brief A base class for entities which have names.
 *
 * Not all entities have names, but those that do (e.g., classes, methods, objects) inherit from this class *as well as* from Entity.
 */
class Named {
	protected:
		std::string name;

		explicit Named(const std::string& name) : name(name) {}
		~Named() = default;
		Named(const Named& other) = default;
		Named(Named&& other) noexcept = default;
		Named& operator=(const Named& other) = default;
		Named& operator=(Named&& other) noexcept = default;

	public:
		Named() = delete;

		const std::string& getName() const { return name; }
		std::string_view viewName() const { return name; }
};

} // namespace bpp::IR::Components
