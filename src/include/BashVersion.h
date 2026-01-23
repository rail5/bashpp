/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <cstdint>
#include <string>

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
