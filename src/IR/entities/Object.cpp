/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Object.h"

#include <error/InternalError.h>

namespace bpp::IR {

std::string Object::getAddress() const {
	bpp_assert(!type.expired(), "Object does not have a type");
	bpp_assert(!name.empty(), "Object does not have a name");

	std::string address = "bpp__";

	if (m_is_pointer) address += "__ptr__";

	address += type.lock()->getName() + "__" + name;

	return address;
}

} // namespace bpp::IR
