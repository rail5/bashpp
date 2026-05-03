/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

namespace bpp {

bpp_method_parameter::bpp_method_parameter(const std::string& name) {
	set_name(name);
}

void bpp_method_parameter::set_class(std::shared_ptr<bpp_class> type) {
	this->type = type;
}

} // namespace bpp
