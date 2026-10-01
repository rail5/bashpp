/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

namespace bpp::IR::Components {

/**
 * @brief A base class for entities which have addresses.
 *
 * Not all entities have addresses, but those that do (e.g., objects, methods) inherit from this class *as well as* from Entity.
 *
 * Because of the importance of addresses in code generation, and the need for deterministic compilation (esp. for dynamic includes),
 * the entity's address must be determinable entirely by the entity's properties without modifying state, or relying on any external state
 * (e.g., the order of compilation, or the order of declaration of entities).
 * Hence the const-ness of the getAddress() method.
 */
class Addressable {
	public:
		virtual std::string getAddress() const = 0;
	protected:
		~Addressable() = default;
		Addressable() = default;
		Addressable(const Addressable&) = default;
		Addressable& operator=(const Addressable&) = default;
		Addressable(Addressable&&) = default;
		Addressable& operator=(Addressable&&) = default;
};

} // namespace bpp::IR::Components
