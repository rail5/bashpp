/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Object.h"

#include <IR/entities/Program.h>

#include <error/InternalError.h>

namespace bpp::IR {

bpp::CodeGen::CodeSegment Object::generate_code() {
	bpp::CodeGen::CodeSegment result;

	auto program = containing_program.lock();
	bpp_assert(program != nullptr, "Object does not have a containing program");

	if (m_is_pointer) {
		bpp_assert(!type.expired(), "Pointer does not have a type");

		if (address.empty()) {
			address = "bpp____ptr__" + std::to_string(program->codegen_state.object_counter++)
				+ "__" + type.lock()->get_name() + "__" + name;
		}

		if (program->codegen_state.should_declare_local()) {
			result.add_main_code("local ");
		}
		result.add_main_code(address + "=");
		if (has_initial_value()) {
			result.egalitarian_merge(initial_value.value()->generate_code());
		}
		result.add_main_code("\n");
	} else {
		// FIXME(@rail5): Handle non-pointers
	}

	return result;
}

} // namespace bpp::IR
