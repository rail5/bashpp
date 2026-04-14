/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

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

	constexpr operator uint32_t() const {
		return (static_cast<uint32_t>(major) << 16) | static_cast<uint32_t>(minor);
	}

	constexpr bool operator>=(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) >= static_cast<uint32_t>(other);
	}

	constexpr bool operator<=(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) <= static_cast<uint32_t>(other);
	}

	constexpr bool operator>(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) > static_cast<uint32_t>(other);
	}

	constexpr bool operator<(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) < static_cast<uint32_t>(other);
	}

	constexpr bool operator==(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) == static_cast<uint32_t>(other);
	}

	constexpr bool operator!=(const BashVersion& other) const {
		return static_cast<uint32_t>(*this) != static_cast<uint32_t>(other);
	}

	constexpr std::string to_string() const {
		return std::to_string(major) + "." + std::to_string(minor);
	}

	constexpr operator std::string() const {
		return to_string();
	}

	constexpr BashVersion() = default;
	constexpr ~BashVersion() = default;
	constexpr BashVersion(const BashVersion&) = default;
	constexpr BashVersion& operator=(const BashVersion&) = default;
	constexpr BashVersion(BashVersion&&) = default;
	constexpr BashVersion& operator=(BashVersion&&) = default;

	constexpr BashVersion(uint16_t major, uint16_t minor) : major(major), minor(minor) {}

	/**
	 * @brief Constructs a BashVersion from a string in the format "major.minor" (e.g., "5.2")
	 *
	 * Also accepts a string with just the major version (e.g., "5"), in which case the minor version defaults to 0.
	 * 
	 * @param version_string The version string to parse
	 * @throws std::invalid_argument if the version string is not in a valid format
	 */
	explicit constexpr BashVersion(std::string_view version_string) {
		uint16_t* current = &major;
		uint16_t value = 0;

		for (const auto& c : version_string) {
			switch (c) {
				case '0' ... '9':
					value = (value * 10) + (c - '0');
					break;
				case '.':
					if (current == &minor)
						throw std::invalid_argument("Invalid Bash version: " + std::string(version_string));
					*current = value;
					current = &minor;
					value = 0;
					break;
				default:
					throw std::invalid_argument("Invalid Bash version: " + std::string(version_string));
			}
		}
		*current = value;

		// If we never encountered a dot, the entire string was the major version
		if (current == &major) minor = 0;
	}
};
