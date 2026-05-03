/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

namespace bpp {

void bash_if::new_branch() {
	conditional_branches.emplace_back("", "");
}

void bash_if::add_condition_code(const std::string& condition_code) {
	conditional_branches.back().first += condition_code;
}

void bash_if::add_branch_code(const std::string& branch_code) {
	conditional_branches.back().second += branch_code;
}

const std::vector<std::pair<std::string, std::string>>& bash_if::get_conditional_branches() const {
	return conditional_branches;
}

} // namespace bpp
