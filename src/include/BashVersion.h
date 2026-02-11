/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <cstdint>
#include <string>

/**
 * @struct BashVersion
 * @brief Represents a Bash version to target for code generation
 *
 * This struct is used to specify the target Bash version for code generation.
 * It is represented as a single unsigned 32-bit integer for efficient comparison.
 * The high 16 bits represent the major version, and the low 16 bits represent the minor version.
 * 
 */
struct BashVersion {
	uint16_t major = 5;
	uint16_t minor = 2;

	operator uint32_t() const {
		return (static_cast<uint32_t>(major) << 16) | static_cast<uint32_t>(minor);
	}

	bool operator>=(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) >= static_cast<uint32_t>(other);
	}

	bool operator<=(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) <= static_cast<uint32_t>(other);
	}

	bool operator>(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) > static_cast<uint32_t>(other);
	}

	bool operator<(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) < static_cast<uint32_t>(other);
	}

	bool operator==(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) == static_cast<uint32_t>(other);
	}

	bool operator!=(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) != static_cast<uint32_t>(other);
	}

	std::string to_string() const {
		return std::to_string(major) + "." + std::to_string(minor);
	}

	operator std::string() const {
		return to_string();
	}
};
