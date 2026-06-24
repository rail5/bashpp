/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Object.h"

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment Object::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp::CodeGen::CodeSegment result;

	if (m_is_pointer) {
		bpp_assert(!type.expired(), "Pointer does not have a type");

		if (!state->object_addresses.contains(shared_from_this())) {
			state->object_addresses[shared_from_this()] =
				"bpp____ptr__" + std::to_string(state->object_counter++)
				+ "__" + type.lock()->get_name()
				+ "__" + name;
		}

		if (state->should_declare_local()) {
			result.add_main_code("local ");
		}
		result.add_main_code(state->object_addresses[shared_from_this()] + "=");
		if (has_initial_value()) {
			result.egalitarian_merge(initial_value.value()->generate_code(state));
		}
		result.add_main_code("\n");
	} else {
		// FIXME(@rail5): Handle non-pointers
	}

	return result;
}

} // namespace bpp::IR
