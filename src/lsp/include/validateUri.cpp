
/**
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "validateUri.h"

#include <stdexcept>

std::string validateUri(const std::string& uri) {
	if (!uri.starts_with("file://")) throw std::invalid_argument("URI must start with 'file://'");
	return uri.substr(7); // Strip the "file://" prefix
}
