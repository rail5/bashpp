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
 * @class bpp_object_reference
 * 
 * @brief An object reference in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_object_reference : public bpp_string {
	private:
		bpp::reference_type reference_type = bpp::reference_type::ref_object;
		std::string array_index;
	public:	
		void set_reference_type(bpp::reference_type reference_type);
		void set_array_index(const std::string& array_index);
		bpp::reference_type get_reference_type() const;
		std::string get_array_index() const;
		bool has_array_index() const;
};

} // namespace bpp
