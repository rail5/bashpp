/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <array>
#include <string>
#include <string_view>
#include <algorithm>

namespace bpp::IR {

/**
 * @var protected_keywords
 * @brief A list of keywords that are reserved and cannot be used as identifiers in Bash++
 */
inline constexpr std::array<std::string_view, 18> protected_keywords = {
	"class", "constructor", "delete", "destructor",
	"dynamic_cast", "include", "include_always", "local",
	"method", "new", "nullptr","private",
	"protected", "public", "super", "this",
	"typeof", "virtual",
};

/**
 * @brief Check if a string matches any of our protected keywords
 * @param keyword The string to check
 */
inline bool is_protected_keyword(const std::string& keyword) {
	return std::ranges::contains(protected_keywords, keyword);
}

/**
 * @brief Check if a string is a valid identifier in Bash++
 * @param identifier The string to check
 * @return true if the string is a valid identifier, false otherwise
 */
inline bool is_valid_identifier(const std::string& identifier) {
	// Verify it's not empty, and not a reserved keyword
	if (identifier.empty() || is_protected_keyword(identifier)) {
		return false;
	}

	// Verify it doesn't contain two consecutive underscores
	if (identifier.contains("__")) return false;

	// Verify it starts with a letter or underscore, and contains only letters, digits, and underscores
	if (!isalpha(identifier[0]) && identifier[0] != '_') {
		return false;
	}

	for (char c : identifier) {
		if (!isalnum(c) && c != '_') {
			return false;
		}
	}

	// If all checks passed, it's a valid identifier
	return true;
}

} // namespace bpp::IR
