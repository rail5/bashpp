/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

namespace bpp {

void bash_case::add_case(const std::string& case_) {
	cases += case_;
}

const std::string& bash_case::get_cases() const {
	return cases;
}

} // namespace bpp
