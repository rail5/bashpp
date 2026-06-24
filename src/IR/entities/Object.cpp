/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Object.h"

#include <error/InternalError.h>

namespace bpp::IR {

std::string Object::get_address() const {
	bpp_assert(!type.expired(), "Object does not have a type");
	bpp_assert(!name.empty(), "Object does not have a name");

	std::string address = "bpp__";

	if (m_is_pointer) address += "__ptr__";

	address += type.lock()->get_name() + "__" + name;

	return address;
}

bpp::CodeGen::CodeSegment Object::generate_code(bpp::CodeGen::CodeGenState* state) const {
	bpp::CodeGen::CodeSegment result;

	if (m_is_pointer) {
		bpp_assert(!type.expired(), "Pointer does not have a type");

		if (state->should_declare_local()) {
			result.add_main_code("local ");
		}
		result.add_main_code(get_address() + "=");
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
