/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

namespace bpp {

void bash_case_pattern::set_pattern(const std::string& pattern) {
	this->pattern = pattern;
}

void bash_case_pattern::set_containing_case(std::shared_ptr<bpp::bash_case> containing_case) {
	this->containing_case = std::move(containing_case);
}

const std::string& bash_case_pattern::get_pattern() const {
	return pattern;
}

std::shared_ptr<bpp::bash_case> bash_case_pattern::get_containing_case() const {
	return containing_case;
}

} // namespace bpp
