/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <memory>

#include "bpp.h"
#include "bpp_string.h"

namespace bpp {

/**
 * @class bpp_object_assignment
 * 
 * @brief An object assignment statement in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_object_assignment : public bpp_string {
	private:
		std::string lvalue;
		std::string rvalue;
		bool lvalue_nonprimitive = false;
		bool rvalue_nonprimitive = false;
		std::shared_ptr<bpp_entity> lvalue_object;
		std::shared_ptr<bpp_entity> rvalue_object;
		bool adding = false;
		bool rvalue_array = false;
	public:
		void set_lvalue(const std::string& lvalue);
		void set_rvalue(const std::string& rvalue);
		void set_lvalue_nonprimitive(bool is_nonprimitive);
		void set_rvalue_nonprimitive(bool is_nonprimitive);
		void set_lvalue_object(std::shared_ptr<bpp_entity> object);
		void set_rvalue_object(std::shared_ptr<bpp_entity> object);
		void set_adding(bool is_adding);
		void set_rvalue_array(bool is_array);

		std::string get_lvalue() const;
		std::string get_rvalue() const;
		bool lvalue_is_nonprimitive() const;
		bool rvalue_is_nonprimitive() const;
		std::shared_ptr<bpp_entity> get_lvalue_object() const;
		std::shared_ptr<bpp_entity> get_rvalue_object() const;
		bool is_adding() const;
		bool rvalue_is_array() const;
};

} // namespace bpp
