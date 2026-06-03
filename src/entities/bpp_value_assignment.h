/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>

#include "bpp.h"
#include "bpp_string.h"

namespace bpp {

/**
 * @class bpp_value_assignment
 * 
 * @brief A value assignment statement in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_value_assignment : public bpp_string {
	private:
		bool nonprimitive_assignment = false;
		std::shared_ptr<bpp_entity> nonprimitive_object;
		bool lvalue_nonprimitive = false;
		bool array_assignment = false;
		bool adding = false;
	public:
		void set_nonprimitive_assignment(bool is_nonprimitive);
		void set_nonprimitive_object(std::shared_ptr<bpp_entity> object);
		void set_lvalue_nonprimitive(bool is_nonprimitive);
		void set_array_assignment(bool is_array);
		void set_adding(bool is_adding);

		bool is_nonprimitive_assignment() const;
		std::shared_ptr<bpp_entity> get_nonprimitive_object() const;
		bool lvalue_is_nonprimitive() const;
		bool is_array_assignment() const;
		bool is_adding() const;
};

} // namespace bpp
