/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

#include "bpp.h"
#include "bpp_string.h"

namespace bpp {

/**
 * @class bpp_dynamic_cast_statement
 * 
 * @brief A dynamic_cast statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when we encounter a `@dynamic_cast` statement in Bash++ code.
 * 
 * It contains a pointer to the class we're casting to, and the address we're casting
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_dynamic_cast_statement : public bpp_string {
	private:
		std::string cast_to;
	public:
		void set_cast_to(const std::string& cast_to);
		std::string get_cast_to() const;
};

/**
 * @class bpp_dynamic_cast_target
 * @brief The target of a dynamic_cast in Bash++
 *
 * This entity type represents the target of a `@dynamic_cast` statement in Bash++ code.
 * That is, the expression holding the type to which we are casting.
 *
 * For example, in the statement:
 * `@dynamic_cast<$target_type> &@object`
 * the `$target_type` expression would be held in a bpp_dynamic_cast_target entity.
 * 
 */
class bpp_dynamic_cast_target : public bpp_string {};

} // namespace bpp
