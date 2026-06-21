/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DataMember.h"

namespace bpp::IR {

void DataMember::add_reference_position(const SymbolPosition& pos) {
	Entity::add_reference_position(pos);
	if (auto parent = parent_datamember.lock()) {
		parent->add_reference_position(pos);
	}
}

} // namespace bpp::IR
