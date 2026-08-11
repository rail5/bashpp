/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

namespace bpp::IR {

/**
 * @brief A base class for entities which have addresses.
 *
 * Not all entities have addresses, but those that do (e.g., objects, methods) inherit from this class *as well as* from Entity.
 */
class AddressableEntity {
	public:
		virtual std::string get_address() const = 0;
	protected:
		~AddressableEntity() = default;
		AddressableEntity() = default;
		AddressableEntity(const AddressableEntity&) = default;
		AddressableEntity& operator=(const AddressableEntity&) = default;
		AddressableEntity(AddressableEntity&&) = default;
		AddressableEntity& operator=(AddressableEntity&&) = default;
};

} // namespace bpp::IR
