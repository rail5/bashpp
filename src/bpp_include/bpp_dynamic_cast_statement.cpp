/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

namespace bpp {

void bpp_dynamic_cast_statement::set_cast_to(const std::string& cast_to) {
	this->cast_to = cast_to;
}

std::string bpp_dynamic_cast_statement::get_cast_to() const {
	return cast_to;
}

} // namespace bpp
