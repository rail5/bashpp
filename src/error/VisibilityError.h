/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdexcept>

namespace bpp::ErrorHandling {

struct VisibilityError : public std::invalid_argument {
	public:
		VisibilityError() : std::invalid_argument("Inaccessible entity") {}
};

} // namespace bpp::ErrorHandling
